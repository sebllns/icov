using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

public class IcovConfiguration : MonoBehaviour
{
    public class InteractionConfig
    {
        public bool DeviceFound { get; set; }

        public InteractionConfig()
        {
            DeviceFound = false;
        }
    };

    public class HMDConfig : InteractionConfig { };
    public class MouseKeyboardConfig : InteractionConfig { };
    public class TouchConfig : InteractionConfig { };

    public enum eInteractionType
    {
        NONE = 0,
        HMD,
        MOUSE_KEYBOARD,
        TOUCH
    }

    public static IcovConfiguration Instance
    {
        get;
        private set;
    }

    public string MediaPath { get; set; }

    private eInteractionType _interactionType = eInteractionType.NONE;
    public eInteractionType InteractionType
    {
        get
        {
            return _interactionType;
        }
        set
        {
            Assert.AreNotEqual(eInteractionType.NONE, value, "Interaction cannot be NONE");
            _interactionType = value;
            switch (_interactionType)
            {
                case eInteractionType.HMD:
                    DeviceConfig = new HMDConfig();
                    break;
                case eInteractionType.MOUSE_KEYBOARD:
                    DeviceConfig = new MouseKeyboardConfig();
                    break;
                case eInteractionType.TOUCH:
                    DeviceConfig = new TouchConfig();
                    break;
                default:
                    throw new System.Exception("Should not be there");
            }
        }
    }
    public InteractionConfig DeviceConfig { get; private set; }

    // Start is called before the first frame update
    void Awake()
    {
        Assert.IsNull(Instance);
        Instance = this;
    }


}
