using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HomeMenuNavigation : MonoBehaviour
{
    public GameObject[] Steps;
    public GameObject PreviousButton;
    public GameObject NextButton;

    private int _current;

    private void Start()
    {
        _current = 0;
        Steps[_current].SetActive(true);
        int i = 1;
        while (i < Steps.Length) Steps[i++].SetActive(false);
        PreviousButton.SetActive(false);
        NextButton.SetActive(true);
    }

    public void OnPrevious()
    {
        if(_current > 0)
        {
            Steps[_current--].SetActive(false);
            Steps[_current].SetActive(true);

            NextButton.SetActive(true);
            PreviousButton.SetActive(_current > 0);
        }

    }


    public void OnNext()
    {
        if(_current < Steps.Length-1)
        {
            Steps[_current++].SetActive(false);
            Steps[_current].SetActive(true);

            PreviousButton.SetActive(true);
            NextButton.SetActive(_current < Steps.Length - 1);
        }
    }

    
}
