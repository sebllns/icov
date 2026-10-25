using UnityEngine;
using System.Collections;
using System.IO;
using System;

public class IcovFileLogHandler : ILogHandler
{
    private FileStream m_FileStream;
    private StreamWriter m_StreamWriter;
    private ILogHandler m_DefaultLogHandler = Debug.unityLogger.logHandler;

    public bool AddTimestamp { get; set; }

    public IcovFileLogHandler(string filename, bool addTimestamp = true)
    {
        AddTimestamp = addTimestamp;
        string filePath = Path.GetFullPath(Path.Combine(Application.persistentDataPath, filename));

        Debug.Log("Log path is " + filePath);

        m_FileStream = new FileStream(filePath, FileMode.OpenOrCreate, FileAccess.ReadWrite);
        m_StreamWriter = new StreamWriter(m_FileStream);

        // Replace the default debug log handler
        Debug.unityLogger.logHandler = this;
    }

    public void LogFormat(LogType logType, UnityEngine.Object context, string format, params object[] args)
    {
        m_DefaultLogHandler.LogFormat(logType, context, format, args);
        format = string.Format("[{0}] {1}", System.DateTime.Now.ToString(), format);
        m_StreamWriter.WriteLine(String.Format(format, args));
        m_StreamWriter.Flush();
    }

    public void LogException(Exception exception, UnityEngine.Object context)
    {
        m_DefaultLogHandler.LogException(exception, context);
    }
}