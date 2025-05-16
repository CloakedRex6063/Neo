using System.Runtime.CompilerServices;

internal static class Internal 
{
    internal static class Log
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Info(string input);
    }
}