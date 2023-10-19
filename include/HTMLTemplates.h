#ifndef __HTMLTemplates__
#define __HTMLTemplates__
#include <pgmspace.h>

const char CONFIG_HTML[] PROGMEM =
    R"(<!DOCTYPE html>
<html lang='en'>

<head>
    <title>ESPTools Config Page</title>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width,initial-scale=1' />
    <style>
        body {
            background-color: #171717;
            font-family: Arial, Helvetica, Sans-Serif;
            color: #fff;
        }

        div {
            background: rgb(255 255 255 / 10%);
            padding: 16px;
            border-radius: 16px;
            max-width: 500px;
        }

        input[type=text],
        select {
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

        input[type=submit] {
            width: 100%;
            background-color: #4CAF50;
            color: white;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        input[type=submit]:hover {
            background-color: #45a049;
        }

        a {
            width: 100%;
            background-color: #b44444;
            color: white;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        p {
            padding: 12px 0;
        }
    </style>
</head>

<body>
    <div>
        <form method='POST' id='form' action='' enctype='multipart/form-data'>
        </form>
        <p>
            <a href="restart">Restart ESP</a>
        </p>
        <p>
            <a href="formatfs">Format Filesystem</a>
        </p>
    </div>
</body>

<script>
    let form = document.getElementById('form');

    function populateForm(jsonData) {
        for (let key in jsonData) {
            form.innerHTML += `<label for="${key}">${key}</label><input type='text' name='${key}' id='${key}' value='${jsonData[key]}'>`;
        }
        form.innerHTML += "<input type='submit' value='Save'>";
    }

    fetch(`${window.origin}/configjson`).then(data => {
        data.json().then(json => {
            populateForm(json);
        });
    });
</script>

</html>)";

#endif