using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class InteractionSelection : MonoBehaviour
{
    public Button NextButton;

    private Button _current;
    // Start is called before the first frame update
    void Awake()
    {
        _current = null;
    }

    void OnEnable()
    {
        NextButton.interactable = _current != null;
    }

    public void OnSelect(Button b)
    {
        if (_current != null) _current.interactable = true;
        b.interactable = false;
        _current = b;
        NextButton.interactable = true;

        if (_current.name.ToLowerInvariant().Contains("hmd"))
        {
            IcovConfiguration.Instance.InteractionType = IcovConfiguration.eInteractionType.HMD;
        }
        else if (_current.name.ToLowerInvariant().Contains("mouse-keyboard"))
        {
            IcovConfiguration.Instance.InteractionType = IcovConfiguration.eInteractionType.MOUSE_KEYBOARD;
        }
        else if (_current.name.ToLowerInvariant().Contains("touch"))
        {
            IcovConfiguration.Instance.InteractionType = IcovConfiguration.eInteractionType.TOUCH;
        }
        else
        {
            throw new System.ArgumentException("device unknown");
        }


        var config_menu = transform.parent.Find("InteractionConfig");
        for (int i = 0; i < config_menu.childCount; i++)
        {
            config_menu.GetChild(i).gameObject.SetActive(config_menu.GetChild(i).name == _current.name);
        }
    }
}
