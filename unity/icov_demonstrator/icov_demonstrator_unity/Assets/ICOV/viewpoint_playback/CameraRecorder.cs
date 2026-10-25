using UnityEngine;
using System.IO;

public class CameraRecorder : MonoBehaviour {

    private bool isRecording = false;
    private string positionsFile = "./positions.txt";
    private StreamWriter writer;

    void Start() {
        // Open the file to record camera positions
        writer = new StreamWriter(positionsFile);
    }

    void Update() {
        // Record the camera position
        writer.WriteLine(Time.time + "," + transform.eulerAngles.x + "," + transform.eulerAngles.y);
    }

    public void StartRecording() {
        // Start recording
        isRecording = true;
    }

    public void StopRecording() {
        // Stop recording and close the file
        isRecording = false;
        writer.Close();
    }
}