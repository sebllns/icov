using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using SimpleFileBrowser;
using System;
using System.IO;

public class IcovFileSelector : MonoBehaviour
{
    public Button NextButton;
    public TMPro.TMP_Text SelectedFile;

    // Start is called before the first frame update
    // Warning: paths returned by FileBrowser dialogs do not contain a trailing '\' character
    // Warning: FileBrowser can only show 1 dialog at a time
    void Start()
    {
        // Set filters (optional)
        // It is sufficient to set the filters just once (instead of each time before showing the file browser dialog), 
        // if all the dialogs will be using the same filters
        FileBrowser.SetFilters(false, new FileBrowser.Filter("Any", ".mp4", ".icov", ".jpg", ".jpeg", ".png"), new FileBrowser.Filter("Videos", ".mp4", ".icov"), new FileBrowser.Filter("Images", ".jpg", ".jpeg", ".png"));

        // Set default filter that is selected when the dialog is shown (optional)
        // Returns true if the default filter is set successfully
        // In this case, set Images filter as the default filter
        FileBrowser.SetDefaultFilter(".mp4");

        // Set excluded file extensions (optional) (by default, .lnk and .tmp extensions are excluded)
        // Note that when you use this function, .lnk and .tmp extensions will no longer be
        // excluded unless you explicitly add them as parameters to the function
        FileBrowser.SetExcludedExtensions(".lnk", ".tmp", ".zip", ".rar", ".exe");
    }

    void OnEnable()
    {
        checkFile();
    }

    void OnDisable()
    {
        NextButton.interactable = true;
    }

    public void Show()
    {
        lockUi().SetActive(true);
        // Show a select folder dialog 
        // onSuccess event: print the selected folder's path
        // onCancel event: print "Canceled"
        // Load file/folder: file, Allow multiple selection: false
        // Initial path: default (Documents), Initial filename: empty
        // Title: "Select Folder", Submit button text: "Select"
        FileBrowser.ShowLoadDialog(onSelectPath,
                                  () => { lockUi().SetActive(false); },
                                  FileBrowser.PickMode.Files, false, null, null, "Select 360 file to play", "Open");


    }

    private void onSelectPath(string[] paths)
    {
        Debug.Log("Open 360 media: " + paths[0]);
        lockUi().SetActive(false);

        SelectedFile.text = Path.GetFullPath(paths[0]);
        checkFile();
    }

    private void checkFile()
    {
        if(File.Exists(SelectedFile.text))
        {
            IcovConfiguration.Instance.MediaPath = SelectedFile.text;
            NextButton.interactable = true;
        }
        else
        {
            NextButton.interactable = false;
        }
    }

    private GameObject lockUi()
    {
        return transform.parent.parent.Find("LockUI").gameObject;
    }
}
