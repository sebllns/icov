using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;

public class IcovHmdDetector : MonoBehaviour
{

    //public GameObject HmdPrefab;

    //private GameObject _hmd;

    private void Start()
    {
        UnityEngine.XR.InputDevices.deviceConnected += InputDevices_deviceConnected;
        UnityEngine.XR.InputDevices.deviceDisconnected += InputDevices_deviceDisconnected;
    }

    private void InputDevices_deviceConnected(UnityEngine.XR.InputDevice device)
    {
        Debug.Log(string.Format("Device found with name '{0}' and role '{1}'", device.name, device.role.ToString()));
        //if (_hmd == null) _hmd = GameObject.Instantiate<GameObject>(HmdPrefab);
    }
    private void InputDevices_deviceDisconnected(UnityEngine.XR.InputDevice device)
    {
        Debug.Log(string.Format("Device removed with name '{0}' and role '{1}'", device.name, device.role.ToString()));
    }

    // Start is called before the first frame update
    //void Start()
    //{
    //    //_hmd = null;
    //}

    // Update is called once per frame
    void Update()
    {

    }
}
