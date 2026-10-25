using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class InteractionBase : MonoBehaviour
{
    public GameObject RotateObject;

    public Vector2 Speed = Vector2.one;

    public Vector2 Angle;

    public abstract Vector2 GetAngleDelta();

    // Update is called once per frame
    public virtual void Update()
    {
            Vector2 delta = GetAngleDelta();
            delta.x *= Speed.x;
            delta.y *= Speed.y;

            Vector3 rot = RotateObject.transform.rotation.eulerAngles + new Vector3(delta.y, delta.x, 0f); 
            rot.x = ClampAngle(rot.x, -60.0f, 60.0f);
            rot.y = ClampAngle(rot.y, -180f, 180f);

            Angle = rot;
            RotateObject.transform.eulerAngles = rot;
    }

    protected float ClampAngle(float angle, float from, float to)
     {
        if(angle < -180.0f)
        {
            angle += 360.0f;
        }

        if(angle > 180.0f)
        {
            angle -= 360.0f;
        }

        if(Mathf.Abs(angle) > 180.0f) return ClampAngle(angle, from, to);

        return Mathf.Clamp(angle, from, to);
     }
}
