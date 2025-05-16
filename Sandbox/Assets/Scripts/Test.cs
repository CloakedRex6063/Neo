using System;
using System.Runtime.InteropServices;

namespace Sandbox
{
    public class Test
    {
        public void TestMethod(string input)
        {
            Internal.LogInfo(input);
        }
    }
}