using System.Runtime.InteropServices;

partial class Application
{
    static void Main()
    {
        Run();
        Console.WriteLine("Test");
    }

    [LibraryImport("libSpaceGame.dylib")]
    public static partial void Run();
}