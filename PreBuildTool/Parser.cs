namespace PreBuildTool;
using System.Text.RegularExpressions;

internal partial class Parser
{
    public Parser(string path)
    {
        _input = File.ReadAllText(path);
    }

    public List<StructInfo> Parse()
    {
        var result = new List<StructInfo>();

        var structRegex = StructRegex();
        var fieldRegex = SerializedFieldRegex();

        foreach (Match m in structRegex.Matches(_input))
        {
            var structInfo = new StructInfo { Name = m.Groups[1].Value };
            var body = m.Groups[2].Value;

            foreach (Match fieldMatch in fieldRegex.Matches(body))
            {
                var isSerialized = fieldMatch.Groups[1].Value;
                var type = fieldMatch.Groups[2].Value;
                var name = fieldMatch.Groups[3].Value;

                structInfo.FieldTypes[name] = type;
                if (!string.IsNullOrEmpty(isSerialized))
                    structInfo.SerializedFields.Add(name);
            }

            if(structInfo.SerializedFields.Count == 0) continue;
            result.Add(structInfo);
        }

        return result;
    }
    
    private readonly string _input;

    [GeneratedRegex(@"struct\s+(\w+)\s*\{([^}]+)\};", RegexOptions.Singleline)]
    private static partial Regex StructRegex();
    [GeneratedRegex(@"(\[\[Serialize\]\])?\s*([\w:<>]+(?:\s*[*&])?)\s+(\w+)\s*(?:=\s*[^;]+)?;", RegexOptions.Multiline)]
    private static partial Regex SerializedFieldRegex();
}