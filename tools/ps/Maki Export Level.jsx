var PAGE_SIZE = 512;
var OVERLAP = 1;

var SAVE_OPTS = new ExportOptionsSaveForWeb;
SAVE_OPTS.format = SaveDocumentType.PNG;
SAVE_OPTS.PNG8 = false;
SAVE_OPTS.quality = 100;

function ltrim(s) {
    return s.replace(/^\s+/, '');
}
function rtrim(s) {
    return s.replace(/\s+$/, '')
}
function trim(s) {
    return ltrim(rtrim(s));
}


function loadXMPLibrary() {
    if(!ExternalObject.AdobeXMPScript) {
        try {
            ExternalObject.AdobeXMPScript = new ExternalObject('lib:AdobeXMPScript');
        } catch (e) {
            alert("Can't load XMP Script Library");
            return false;
        }
    }
    return true;
}

function nextPowerOf2(n) {
    return Math.pow(2, Math.ceil(Math.log(n) / Math.LN2));
}

function Buffer() {
    this.buffer = "";
}
Buffer.prototype.write = function(s, depth) {
    indent = "";
    for(var i = 0; i < depth; i++) {
        indent += "\t";
    }
    this.buffer += indent + s + "\n";
};

function pasteInPlace() {
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

    var docWidthPx = doc.width.as("px");
    var docHeightPx = doc.height.as("px");

    var buffer = new Buffer();

    // Record layer visibilities at start
    var oldVisibilities = [];
    for(var li = 0; li < doc.layers.length; li++) {
        var layer = doc.layers[li];
        oldVisibilities.push(layer.visible);
        layer.visible = false;
    }
    
    for(var li = doc.layers.length-1; li >= 0; li--) {
        var layer = doc.layers[li];
        doc.activeLayer = layer;
        layer.visible = true;
        
        // Find tight bounds on the contents of this layer
        var lb = layer.bounds;
        var layerBounds = [Math.max(lb[0].as("px"), 0), Math.max(lb[1].as("px"), 0), Math.min(lb[2].as("px"), docWidthPx), Math.min(lb[3].as("px"), docHeightPx)];
        var layerWidth = layerBounds[2] - layerBounds[0];
        var layerHeight = layerBounds[3] - layerBounds[1];
    
        var xMax = Math.ceil(layerWidth / (PAGE_SIZE-OVERLAP));
        var yMax = Math.ceil(layerHeight / (PAGE_SIZE-OVERLAP));
    
        buffer.write("layer", 0);
        buffer.write("name \"" + layer.name + "\"", 1);
        
        // Parse layer metadata
        try {
            xmp = new XMPMeta(layer.xmpMetadata.rawData);
        } catch(e) {
            xmp = new XMPMeta();
        }
        var metaDoc = trim(xmp.getProperty("http://makiengine.com/", "data", XMPConst.STRING).toString());
        metaDoc = metaDoc.replace(/%0d/g, '\r');
        metaDoc = metaDoc.replace(/%0a/g, '\n');
        metaDoc = metaDoc.replace(/%09/g, '\t');
        metaDoc = metaDoc.replace(/%25/g, '%');
        if(metaDoc.length > 0) {
            buffer.write("meta", 1);
            var lines = metaDoc.split('\n');
            for(var i = 0; i < lines.length; i++) {
                var s = rtrim(lines[i]);
                if(s.length > 0) {
                    buffer.write(s, 2);
                }
            }
        }      
        
        // This layer rect is relative to the stage
        buffer.write("pos " + layerBounds[0] + ", " + layerBounds[1], 1);
        buffer.write("tiles", 1);
        
        for(var x = 0; x < xMax; x++) {
            for(var y = 0; y < yMax; y++) {
                    
                    var left = layerBounds[0] + x*(PAGE_SIZE-OVERLAP);
                    var top = layerBounds[1] + y*(PAGE_SIZE-OVERLAP);
                    var right = Math.min(left+PAGE_SIZE, layerBounds[2]);
                    var bottom = Math.min(top+PAGE_SIZE, layerBounds[3]);
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
                    
                    var scratchDoc = app.documents.add(w, h);
                    var l = scratchDoc.artLayers.add();
                    l.kind = LayerKind.NORMAL;
                    scratchDoc.backgroundLayer.remove();
                    //scratchDoc.selection.select([[0, 0], [0, h], [w, h], [w, 0]]);
                    //scratchDoc.selection.selectAll();
                    //scratchDoc.paste(true);
                    pasteInPlace();
                    

                    // Trim transparent pixels, and figure out how many we trimmed off the left and top,
                    // since we'll have to offset the final rect by that amount
                    scratchDoc.trim(TrimType.TRANSPARENT, false, false, true, true);
                    var oldWidth = scratchDoc.width.as("px");
                    var oldHeight = scratchDoc.height.as("px");                    
                    scratchDoc.trim(TrimType.TRANSPARENT, true, true, false, false);
                    var trimmedWidth = scratchDoc.width.as("px");
                    var trimmedHeight = scratchDoc.height.as("px");
                    
                    left += oldWidth - trimmedWidth;
                    top += oldHeight - trimmedHeight;

                    scratchDoc.resizeCanvas(nextPowerOf2(trimmedWidth), nextPowerOf2(trimmedHeight), AnchorPosition.TOPLEFT);
                    
                    var f = new File(fileName);
                    scratchDoc.exportDocument(f, ExportType.SAVEFORWEB, SAVE_OPTS);
                    f.close();
                    scratchDoc.close(SaveOptions.DONOTSAVECHANGES);
                    
                    buffer.write("tile", 2);
                    buffer.write("path \"" + fileName + "\"", 3);

                    // This rectangle is relative to the layer rect
                    buffer.write("rect " + (left - layerBounds[0]) + ", " + (top - layerBounds[1]) + ", " + trimmedWidth + ", " + trimmedHeight, 3);
            }
        }
    
        layer.visible = false;
    }

    // Set layer visibilities back to the way they were
    for(var li = 0; li < doc.layers.length; li++) {
        var layer = doc.layers[li];
        layer.visible = oldVisibilities[li];
    }


    if(file.open("w")) {
        file.write(buffer.buffer);
        file.close();
        return "Ok";
    } else {
        return "Failed to open output file";
    }
}



loadXMPLibrary();
var metaFile = File.saveDialog("Save level as", "Maki Level:*.mphot");
if(metaFile == null) {
    "User cancelled";
} else {
    $.writeln("Exporting to: " + metaFile.fullName);
    exportLevel(metaFile);
}
