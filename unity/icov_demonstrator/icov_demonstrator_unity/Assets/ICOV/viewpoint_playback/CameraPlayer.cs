using UnityEngine;
using System.IO;

public class CameraPlayer : MonoBehaviour {

    public bool isPlaying = false;
    private string[] positions;
    private int positionIndex = 0;
    private float startTime = 0;

    void Start() {
        // Load the recorded camera positions from the file
        string positionsText = File.ReadAllText("./positions.txt");
        positions = positionsText.Split('\n');
    }

    void Update() {
        // Check if we are in playback mode
        if (isPlaying) {
            // Check if we have reached the end of the recorded positions
            if (positionIndex >= positions.Length) {
                isPlaying = false;
                return;
            }

            // Read the recorded positions and move the camera accordingly
            try
            {
            string[] pos = positions[positionIndex].Split(',');
            float time = float.Parse(pos[0]);
            if (startTime == 0) {
                startTime = Time.time;
            }
            float t = (Time.time - startTime) / time;
            transform.eulerAngles = new Vector3(float.Parse(pos[1]), float.Parse(pos[2]), 0);
            positionIndex++;
            }catch(System.Exception){

            }

            return;
        }
    }

    public void StartPlaying() {
        // Start playback
        isPlaying = true;
        startTime = 0;
        positionIndex = 0;
    }

    public void StopPlaying() {
        // Stop playback
        isPlaying = false;
    }
}