var PAGE_SIZE = 512;

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

function pasteInto() {
    var idpast = charIDToTypeID( "past" );
    var desc557 = new ActionDescriptor();
    var idinPlace = stringIDToTypeID( "inPlace" );
    desc557.putBoolean( idinPlace, true );
    var idAntA = charIDToTypeID( "AntA" );
    var idAnnt = charIDToTypeID( "Annt" );
    var idAnno = charIDToTypeID( "Anno" );
    desc557.putEnumerated( idAntA, idAnnt, idAnno );
    executeAction( idpast, desc557, DialogModes.NO );
}

function exportLevel(file) {
    var doc = app.activeDocument;
    var dot = file.fsName.lastIndexOf(".");
    if(dot == -1) {
        dot = file.fsName.length;
    }
    var baseName = metaFile.fsName.slice(0, dot);

    var xMax = Math.ceil(doc.width / PAGE_SIZE);
    var yMax = Math.ceil(doc.height / PAGE_SIZE);    
    
    write(file, "size " + doc.width.as("px") + ", " + doc.height.as("px"), 0);
    write(file, "layers", 0);
    
    // Record layer visibilities at start
    var oldVisibilities = [];
    for(var li = 0; li < doc.layers.length; li++) {
        var layer = doc.layers[li];
        oldVisibilities.push(layer.visible);
        layer.visible = false;
    }
    
    for(var li = 0; li < doc.layers.length; li++) {
        var layer = doc.layers[li];
        doc.activeLayer = layer;
        layer.visible = true;
        
        write(file, "layer", 1);
        write(file, "name \"" + layer.name + "\"", 2);
        write(file, "tiles", 2);
        
        for(var x = 0; x < xMax; x++) {
            for(var y = 0; y < yMax; y++) {
                    
                    var left = x*PAGE_SIZE;
                    var top = y*PAGE_SIZE;
                    var right = left+PAGE_SIZE;
                    var bottom = top+PAGE_SIZE;
                    var w = right - left;
                    var h = bottom - top;
                    
                    var beforeCrop = doc.activeHistoryState;
                                        
                    doc.crop([left, top, right, bottom]);
                    doc.selection.select([[0, 0], [0, h], [w, h], [w, 0]]);
                                        
                    // An exception will be thrown if the selection is empty
                    try {
                        doc.selection.copy(true);
                    } catch(e) {
                        doc.activeHistoryState = beforeCrop;
                        continue;
                    }
                    // Rollback changes
                    doc.activeHistoryState = beforeCrop;
                    
                    var fileName = baseName + "_" + li + "_" + x + "_" + y + ".png";
                    $.writeln("Saving: " + fileName);
                    
                    var scratchDoc = app.documents.add(PAGE_SIZE, PAGE_SIZE);
                    var l = scratchDoc.artLayers.add();
                    l.kind = LayerKind.NORMAL;
                    scratchDoc.backgroundLayer.remove();
                    scratchDoc.selection.select([[0, 0], [0, h], [w, h], [w, 0]]);
                    pasteInto();

                    // Trim transparent pixels, and figure out how many we trimmed off the left and top,
                    // since we'll have to offset the final rect by that amount
                    var oldWidth = scratchDoc.width.as("px");
                    var oldHeight = scratchDoc.height.as("px");
                    scratchDoc.trim(TrimType.TRANSPARENT, true, true, false, false);
                    var trimmedWidth = scratchDoc.width.as("px");
                    var trimmedHeight = scratchDoc.height.as("px");
                    left += oldWidth - trimmedWidth;
                    top += oldHeight - trimmedHeight;
                    scratchDoc.trim(TrimType.TRANSPARENT, false, false, true, true);
                    
                    trimmedWidth = scratchDoc.width.as("px");
                    trimmedHeight = scratchDoc.height.as("px");
                    var texWidth = nextPowerOf2(trimmedWidth);
                    var texHeight = nextPowerOf2(trimmedHeight);
                    scratchDoc.resizeCanvas(texWidth, texHeight, AnchorPosition.TOPLEFT);
                    
                    var f = new File(fileName);
                    scratchDoc.exportDocument(f, ExportType.SAVEFORWEB, SAVE_OPTS);
                    f.close();
                    scratchDoc.close(SaveOptions.DONOTSAVECHANGES);
                    
                    write(file, "tile", 3);
                    write(file, "path \"" + fileName + "\"", 4);
                    write(file, "size " + texWidth + ", " + texHeight, 4);
                    write(file, "rect " + left + ", " + top + ", " + trimmedWidth + ", " + trimmedHeight, 4);
            }
        }
    
        layer.visible = false;
    }

    // Set layer visibilities back to the way they were
    for(var li = 0; li < doc.layers.length; li++) {
        var layer = doc.layers[li];
        layer.visible = oldVisibilities[li];
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
