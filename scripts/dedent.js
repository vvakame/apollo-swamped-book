let fs = require("fs");
let fileName = "./articles/utilities.re";

let content = fs.readFileSync("./articles/utilities.re", { encoding: "utf8" });
content = content.replace(/^\s{4}/gm, "");
fs.writeFileSync(fileName, content);
