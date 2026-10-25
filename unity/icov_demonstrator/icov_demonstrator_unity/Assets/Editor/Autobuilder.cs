// C# example
using System.IO;
using UnityEditor;
class Autobuilder
{
     [MenuItem ("File/AutoBuilder/Android")]
     static void PerformBuildAndroid ()
     {
         string[] scenes = { "Assets/icov.unity" };

         string buildPath = "../../build/client/android";

         // Create build folder if not yet exists
         Directory.CreateDirectory(buildPath);

         BuildPipeline.BuildPlayer(scenes, buildPath + "/player.apk", BuildTarget.Android, BuildOptions.None);
     }
}
