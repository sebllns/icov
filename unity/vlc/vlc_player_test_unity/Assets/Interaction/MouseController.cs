using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class MouseController : InteractionBase
{
    public Text DebugText;

    private Vector2 _clickPosition;
    // Start is called before the first frame update
    void Start()
    {
    }

    public override Vector2 GetAngleDelta()
    {

        DebugText.text = string.Format("V: {0}\nH: {1}", Angle.x, Angle.y);

        if(Input.GetMouseButtonDown(0))
        {
            _clickPosition = Input.mousePosition;
        }

        if(Input.GetMouseButton(0))
        {
            Vector2 mousePos = Input.mousePosition;
            Vector2 delta = mousePos - _clickPosition;
            Vector2 maxDelta = 0.2f * new Vector2(Screen.width, Screen.height);


            delta.x = Mathf.Clamp(Mathf.Sign(delta.x)* Mathf.Pow(Mathf.Abs(delta.x)/maxDelta.x, 1.2f), -1.0f, 1.0f);
            delta.y = Mathf.Clamp(Mathf.Sign(delta.y)* Mathf.Pow(Mathf.Abs(delta.y)/maxDelta.y, 1.2f), -1.0f, 1.0f);

            //Debug.Log(delta);

            return delta;
        
        }

        return Vector2.zero;
    }


}
