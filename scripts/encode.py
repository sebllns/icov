import zipfile
import pathlib
import os
import sys
import argparse
import subprocess

# parser = argparse.ArgumentParser(description='Process some integers.')
# parser.add_argument('integers', metavar='N', type=int, nargs='+',
#                     help='an integer for the accumulator')
# parser.add_argument('--sum', dest='accumulate', action='store_const',
#                     const=sum, default=max,
#                     help='sum the integers (default: find the max)')

# args = parser.parse_args()
# print(args.accumulate(args.integers))

#icov_bin_dir = sys.argv[1]
#icov_output_dir = sys.argv[2]

icov_bin_dir = '/home/sbelleno/_ICOV/CODE/adt.git/build/release/bin'
icov_gop_size = 16
icov_gop_count = 2

icov_bin_name = 'icov_cli'
icov_bin = os.path.join(icov_bin_dir, icov_bin_name)
icov_log_level = 1
icov_qp = 32
icov_log_data = os.path.join(icov_bin_dir, "data/imt_164_v2/")
icov_source_video = "/home/sbelleno/NOSAVE/icov/data2/360_Timelapse_Bled_Lake_in_Slovenia.webm"
icov_file = "/home/sbelleno/NOSAVE/icov/test_gops.icov"
icov_input_gops = os.path.join(icov_file, "input_gops")
icov_output_gops = os.path.join(icov_file, "output_gops")
icov_gop_size = 16
icov_gop_count = 2

icov_make_gop_args = [icov_bin, 
             "--log-level", str(icov_log_level), 
             "--encode-icu-size", "32", 
             "--mode", "extract-gop" ,
             "--gop-size", str(icov_gop_size),
             "--gop-count", str(icov_gop_count), 
             "-i", icov_source_video, 
             "-o", icov_input_gops]

subprocess.run(icov_make_gop_args)

icov_encode_args = [icov_bin, 
             "--log-level", str(icov_log_level), 
             "--ldpc-coder-type", "imt164", 
             "--ldpc-data-path", icov_log_data, 
             "--encode-qp", str(icov_qp), 
             "--encode-icu-size", "32", 
             "--mode", "encode", 
             "-i", icov_input_gops, 
             "-o", icov_output_gops, 
             "--encode-viewport-image-ratio", "1.7778", 
             "--encode-viewport-erp-horizontal-ratio", "0.4"]

p = subprocess.Popen(icov_encode_args)

p.communicate()


# with zipfile.ZipFile(icov_file + '.icov.zip', mode="w") as archive:
#     #archive.write(icov_file + '.info')
#     for file_path in pathlib.Path(icov_file + '.icov/').iterdir():
#         archive.write(file_path)


# import shutil
# def zip_folder_with_shutil(source_folder, output_path):
#    shutil.make_archive(output_path, 'zip', source_folder)


# def main(argv):
#    inputfile = 'tmp.jpg'
#    outputfile = 'tmp.icov'
#    opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
#    for opt, arg in opts:
#       if opt == '-h':
#          print ('encode.py -i <inputfile> -o <outputfile>')
#          sys.exit()
#       elif opt in ("-i", "--ifile"):
#          inputfile = arg
#       elif opt in ("-o", "--ofile"):
#          outputfile = arg
#    print ('Input file is ', inputfile)
#    print ('Output file is ', outputfile)

# if __name__ == "__main__":
#    main(sys.argv[1:])       