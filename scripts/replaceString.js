let fs = require("fs");

{
    let fileName = "./articles/introduction.re";
    let content = fs.readFileSync(fileName, { encoding: "utf8" });
    content = content.replace(/✔/gm, "✓"); // Apolloの出力そのままだとTofuになる
    fs.writeFileSync(fileName, content);
}
{
    let fileName = "./articles/utilities.re";
    let content = fs.readFileSync(fileName, { encoding: "utf8" });
    content = content.replace(/^\s{4}/gm, "");
    fs.writeFileSync(fileName, content);
}
