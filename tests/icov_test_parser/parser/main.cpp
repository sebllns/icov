#include "video_layer/nal/NalUnit.h"
#include "video_layer/nal/ViewportUnit.h"
#if _MSC_VER
#define _HAS_STD_BYTE 0
#include <Windows.h>
#endif

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "video_layer/extracted_stream.h"

#include "tools/Logger.h"

#include "extensions/bitstream-utils/BitstreamExtension.h"

namespace fs = std::filesystem;

using namespace std;
using namespace icov;
using namespace BitstreamExtension;

static unsigned char *s_data;
static std::streambuf *s_sink;

void printlog(const char *message, int length) {
  std::ostream str(s_sink);
  str << "REDIRECTED LOG " << message;
}

int main(int argc, char *argv[]) {

  icov::Logger log(cout);
  s_sink = log.getSink();
  log.showTimestamp = true;

  log.logToFile("log.txt");
  // log.redirect(&printlog);

  // I - READ IMAGE
  cout << "I - READ IMAGE" << endl;

  int image_width = 512;
  int image_height = 256;
  int pixel_depth = 1;

  int block_size = 32;
  int block_pixel_count = block_size * block_size;

  int image_pixel_count = image_width * image_height * pixel_depth;
  int block_count = image_pixel_count / block_pixel_count;

  s_data = nullptr;

  char *data = new char[image_pixel_count];

  cout << "read file" << endl;

  ifstream fin;
  fin.open("test.icov", ios::binary | ios::in);
  fin.read(data, image_pixel_count);
  fin.close();

  s_data = (unsigned char *)data;

  cout << "-- loaded: " << image_width << " * " << image_height << " * "
       << pixel_depth << std::endl;

  // II - ENCAPSULATE NAL
  cout << "II - ENCAPSULATE NAL" << endl;
  bitstream::ViewportUnit vpu;

  for (int k = 0; k < block_count; k++) {
    bitstream::byte *icuData = new bitstream::byte[block_pixel_count];
    memcpy(icuData, s_data + k * block_pixel_count, block_pixel_count);
    vpu.addIcu(icov::bitstream::InteractiveCodedUnit(icuData, block_size));
  }

  // III - SERIALIZE NAL
  cout << "III - SERIALIZE NAL" << endl;

  bitstream::NalUnit nalu = vpu.ToNal();
  bitstream::byte *nalu_bytes = nullptr;

  nalu.serialize(nalu_bytes);

  // IV - DESERIALIZE NAL
  cout << "IV - DESERIALIZE NAL" << endl;
  bitstream::NalUnit nalu_to_parse;
  nalu_to_parse.deserialize(nalu_bytes);

  bitstream::ViewportUnit vpu_to_parse;
  vpu_to_parse.TryParse(nalu_to_parse);

  // V - WRITE FILE BACK AND COMPARE CHECKSUM
  cout << "V - WRITE FILE BACK TO COMPARE" << endl;

  std::fstream fparsed;
  fparsed.open("test_parsed.dat",
               fstream::in | fstream::out | fstream::trunc | fstream::binary);

  int parsed_bytecount = 0;

  fparsed.seekg(0);

  for (int i = 0; i < vpu_to_parse.getIcuCount(); i++) {
    int icu_size = vpu_to_parse.getIcus()[i].GetDataLength();
    icu_size *= icu_size;
    parsed_bytecount += icu_size;
    fparsed.write((char *)vpu_to_parse.getIcus()[i].GetEncodedData(), icu_size);
    // std::cout << icu_size << std::endl;
    fparsed.flush();
  }

  char *parsed_data = new char[parsed_bytecount];
  fparsed.flush();
  fparsed.seekg(0);
  fparsed.read(parsed_data, parsed_bytecount);
  fparsed.close();

  auto checksum_orig = BitstreamExtension::GetCheckSum(
      (const bitstream::byte *)parsed_data, parsed_bytecount);
  auto checksum_parsed = BitstreamExtension::GetCheckSum(
      (const bitstream::byte *)data, image_pixel_count);

  bool checksum_equal =
      BitstreamExtension::ChecksumEqual(checksum_orig, checksum_parsed);
  assert(checksum_equal);
  std::cout << "Checksums are equal: " << log_condition(checksum_equal) << endl;

  return BitstreamExtension::ChecksumDiff(checksum_orig, checksum_parsed);
}
