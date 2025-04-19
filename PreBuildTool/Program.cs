using PreBuildTool;

if (args.Length < 2)
{
    Console.WriteLine("Usage: PreBuildTool.exe <project-directory> <target-path>");
    return;
}

List<StructInfo> parsedStructInfos = [];

foreach (var file in Directory.GetFiles(args[0], "*.hpp", SearchOption.AllDirectories))
{
    var parser = new Parser(file);
    parsedStructInfos.AddRange(parser.Parse());
}

var metadata = new Metadata(parsedStructInfos);
var generatedCode = metadata.Generate();
File.WriteAllText(args[1], generatedCode);

public class StructInfo
{
    public string Name = "";
    public readonly List<string> SerializedFields = [];
    public readonly Dictionary<string, string> FieldTypes = [];
}