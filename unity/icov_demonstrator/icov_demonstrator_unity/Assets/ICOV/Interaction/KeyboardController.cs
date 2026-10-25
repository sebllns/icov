using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class KeyboardController : InteractionBase
{
    // Start is called before the first frame update
    void Start()
    {
    }

    public override Vector2 GetAngleDelta()
    {
        return new Vector2(Input.GetAxis("Horizontal"), Input.GetAxis("Vertical"));
    }


}
