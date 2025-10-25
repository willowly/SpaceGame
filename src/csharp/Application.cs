using System.Runtime.InteropServices;

partial class Application
{
    static void Main()
    {
        Run();
    }

    [LibraryImport("libSpaceGame.dylib")]
    public static partial void Run();
}