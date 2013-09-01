var PAGE_SIZE = 2048;

var SAVE_OPTS = new ExportOptionsSaveForWeb();
SAVE_OPTS.format = SaveDocumentType.PNG;
SAVE_OPTS.PNG8 = false;
SAVE_OPTS.quality = 100;

function nextPowerOf2(n) {
    return Math.pow(2, Math.ceil(Math.log(n) / Math.LN2));
}

function write(file, s, depth) {
    indent = "";
    for(var i = 0; i < depth; i++) {
        indent += "\t";
    }
    file.writeln(indent + s);
}

function exportLevel(file) {
    var doc = app.activeDocument;
    var dot = file.fsName.lastIndexOf(".");
    if(dot == -1) {
        dot = file.fsName.length;
    }
    var baseName = metaFile.fsName.slice(0, dot);
    
    var docWidthPx = doc.width.as("px");
    var docHeightPx = doc.height.as("px");
    
    write(file, "size " + docWidthPx + ", " + docHeightPx, 0);
    write(file, "images", 0);

    var xMax = Math.ceil(doc.width / PAGE_SIZE);
    var yMax = Math.ceil(doc.height / PAGE_SIZE);
    for(var x = 0; x < xMax; x++) {
        for(var y = 0; y < yMax; y++) {
                
                var left = x*PAGE_SIZE;
                var top = y*PAGE_SIZE;
                var right = Math.min(left+PAGE_SIZE, docWidthPx);
                var bottom = Math.min(top+PAGE_SIZE, docHeightPx);
                var w = right - left;
                var h = bottom - top;
                
                var srcRegion = [[left, top], [left, bottom], [right, bottom], [right, top]];
                var dstRegion = [[0, 0], [0, h], [w, h], [w, 0]];
                
                var pow2Width = nextPowerOf2(w);
                var pow2Height = nextPowerOf2(h);
                
                var fileName = baseName + "_" + x + "_" + y + ".png";                
                $.writeln("Saving: " + fileName);

                write(file, "image", 1);
                write(file, "path \"" + fileName + "\"", 2);
                write(file, "size " + pow2Width + ", " + pow2Height, 2);
                write(file, "rect " + left + ", " + top + ", " + w + ", " + h, 2);
                
                doc.selection.select(srcRegion);
                doc.selection.copy(true);
                
                var scratchDoc = app.documents.add(pow2Width, pow2Height);
                var backgroundLayer = scratchDoc.layers[0];
                var layer = scratchDoc.artLayers.add();
                layer.kind = LayerKind.NORMAL;
                backgroundLayer.remove();
                
                scratchDoc.selection.select(dstRegion);
                scratchDoc.paste(true);
                                
                var f = new File(fileName);
                scratchDoc.exportDocument(f, ExportType.SAVEFORWEB, SAVE_OPTS);
                f.close();
                
                scratchDoc.close(SaveOptions.DONOTSAVECHANGES);
        }
    }
}



var metaFile = File.saveDialog("Save level as", "Maki Level:*.mphot");
if(metaFile == null) {
    "User cancelled";
} else {
    if(metaFile.open("w")) {
        $.writeln("Exporting to: " + metaFile.fullName);
        exportLevel(metaFile);
        metaFile.close();
        "Ok";
    } else {
        throw "Failed to open output file";
    }
}
