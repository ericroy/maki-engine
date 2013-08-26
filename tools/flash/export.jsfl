
var PI = 3.1415926535897932384626433832795;

function fopen(uri, debugTrace) {
	return {
		uri: uri,
		buffer: "",
		debugTrace: debugTrace,
		write: function(s, depth) {
			indent = "";
			if(typeof(depth) != "undefined") {
				for(var i = 0; i < depth; i++) {
					indent += "\t";
				}
			}
			if(this.debugTrace) {
				fl.trace("<<<<" + indent + s);
			}
			this.buffer += indent + s + "\r\n";
		},
		writePair: function(key, value, depth, quote) {
			var addQuotes = false;
			if(typeof(quote) != "undefined" && quote == true) {
				addQuotes = true;
			}
			var s = key + " ";
			if(addQuotes) {
				s += "\"" + value + "\"";
			} else {
				s += value;
			}
			this.write(s, depth);
		},
		close: function() {
			FLfile.write(this.uri, this.buffer);
		}
	};
}

function splitIntegerPair(s) {
	var parts = s.toString().split(",");
	parts[0] = parseInt(parts[0]);
	parts[1] = parseInt(parts[1]);
	return parts;
}
	

function writeBezier(elem, fp, depth) {
	if(typeof(elem) == "undefined" || elem.@enabled != "1") {
		return;
	}
	fp.write("\"" + elem.@id.toLowerCase() + "\"", depth);
	
	var keyframes = elem.Keyframe;	
	for(var i = 0; i < keyframes.length(); i++) {
		var kf = keyframes[i];
		
		var anchor = splitIntegerPair(kf.@anchor);
		if(anchor[0] != 0) {
			throw "Expected anchor point x value to always be zero";
		}
		var prev = splitIntegerPair(kf.@previous);
		var next = splitIntegerPair(kf.@next);
		var time = parseInt(kf.@timevalue.toString());
		
		// Time values for prev and next handles are relative to anchor time
		prev[0] += time;
		next[0] += time;

		fp.writePair("control_point", prev[0] + ", " + prev[1] + ", " + anchor[0] + ", " + anchor[1] + ", " + next[0] + ", " + next[1], depth+1);
	}
}

function padNumber(n, length) {
	var s = "";
	for(var i = 0; i < length; i++) {
		s += "0";
	}
	s += n;
	return s.slice(-length);
}

function exportScene(uri, debugTrace, leaveMetaDataOnDisk) {
	var pathChunks = uri.split("/");
	var exportDir = pathChunks.slice(0, pathChunks.length-1).join("/");
	fl.trace(exportDir);
	
	var dom = fl.getDocumentDOM();
	var timeline = dom.getTimeline();
	var maxFrameCount = timeline.frameCount;
	var layerCount = timeline.layerCount;

	var originalFrame = timeline.currentFrame;
	var originalLayer = timeline.currentLayer;

	var referencedLibraryItems = {};
	var libraryList = [];
	var elementIds = {};
	var nextElementId = 0;
	
	var fp = fopen(uri, debugTrace);
	fp.writePair("frame_rate", dom.frameRate);
	fp.writePair("max_frame_count", maxFrameCount);
	fp.write("layers");

	for(var li = 0; li < layerCount; li++) {
		// Have the IDE select this layer
		timeline.currentLayer = li;
		var layer = timeline.layers[li];
		
		fp.write("layer", 1);
		fp.writePair("name", layer.name, 2, true);
		fp.writePair("visible", layer.visible, 2);
		fp.writePair("outline", layer.outline, 2);
		
		var exportLibAssets = layer.visible && !layer.outline;
		
		fp.write("key_frames", 2);
		for(var fi = 0; fi < layer.frames.length;) {
			var frame = layer.frames[fi];
			if(fi != frame.startFrame) {
				continue;
			}
			
			// Have the IDE select this frame
			timeline.currentFrame = fi;
			
			fp.write("key_frame", 3);
			fp.writePair("frame_start", frame.startFrame, 4);
			fp.writePair("frame_duration", frame.duration, 4);
			
			if(frame.isMotionObject() && frame.hasMotionPath()) {
				fp.write("tween", 4);
				//fp.writePair("type", frame.tweenType, 4, true);
						
				var xml = new XML(frame.getMotionObjectXML());
				fp.writePair("time_scale", xml.@TimeScale, 5);
				fp.writePair("time_duration", xml.@duration, 5);
				fp.writePair("easing", xml.TimeMap.@type.toString().toLowerCase(), 5, true);
				fp.writePair("ease_strength", xml.TimeMap.@strength, 5);
				
				fp.write("properties", 5);
				writeBezier(xml..Property.(@id=="Motion_X"), fp, 6);
				writeBezier(xml..Property.(@id=="Motion_Y"), fp, 6);
				writeBezier(xml..Property.(@id=="Scale_X"), fp, 6);
				writeBezier(xml..Property.(@id=="Scale_Y"), fp, 6);
			}
			
			fp.write("elements", 4);			
			for(var ei = 0; ei < frame.elements.length; ei++) {
				var e = frame.elements[ei];
				if(e.elementType != "instance") {
					fl.trace("Unsupported element type: " + e.elementType);
					continue;
				}
				if(e.instanceType != "symbol" && e.instanceType != "bitmap") {
					fl.trace("Unsupported instance type: " + e.instanceType);
					continue;
				}

				var subtype = "";
				if(e.instanceType == "symbol") {
					subtype = e.symbolType;
				}
				
				fp.write("element", 5);
				if(e.name.length > 0) {
					fp.writePair("name", e.name, 6, true);
				}
				
				//fp.writePair("type", "\"" + e.instanceType + "\", \"" + subtype + "\"", 5);
				fp.writePair("z_index", e.depth, 6);
				fp.writePair("size", e.width + ", " + e.height, 6);
				fp.writePair("theta", -e.rotation * PI / 180.0, 6);
				fp.writePair("reg_point", e.x + ", " + e.y, 6);
				fp.writePair("trans_point", e.transformX + ", " + e.transformY, 6);
				fp.writePair("matrix", e.matrix.a + ", " + e.matrix.b + ", " + e.matrix.c + ", " + e.matrix.d + ", " + e.matrix.tx + ", " + e.matrix.ty, 6);
				
				var libIndex = -1;
				if(exportLibAssets) {
					if(e.libraryItem.name in referencedLibraryItems) {
						libIndex = referencedLibraryItems[e.libraryItem.name];
					} else {
						libIndex = libraryList.length;
						referencedLibraryItems[e.libraryItem.name] = libIndex;
						libraryList.push(e.libraryItem.name);
					}
				}
				fp.writePair("library_index", libIndex, 6);
			}

			fi += frame.duration;
		}
	}

	var lib = document.library;

	var sheet = new SpriteSheetExporter();
	sheet.layoutFormat = "JSON";
	sheet.borderPadding = 2;	
	sheet.shapePadding = 2;
	sheet.stackDuplicateFrames = true;
	var sheetUri = exportDir + "/" + pathChunks[pathChunks.length-1].split(".")[0];
	libraryList.forEach(function(itemName) {
		var item = lib.items[lib.findItemIndex(itemName)];
		sheet.addSymbol(item);
	});
	var meta = sheet.exportSpriteSheet(sheetUri, {format: "png", bitDepth:32, backgroundColor: "#00000000"}, leaveMetaDataOnDisk);
	meta = eval("(" + meta + ")");
	if(!leaveMetaDataOnDisk) {
		FLfile.remove(sheetUri + ".json");
	}
	
	fp.write("sprite_sheets");
	fp.write("sheet", 1);
	fp.writePair("size", meta.meta.size.w + ", " + meta.meta.size.h, 2);
	//fp.writePair("format", meta.meta.format, 2, true);
	fp.writePair("path", FLfile.uriToPlatformPath(sheetUri+".png"), 2, true);
	
	fp.write("library");
	libraryList.forEach(function(itemName) {
		var item = lib.items[lib.findItemIndex(itemName)];
		fp.write("item", 1);
		fp.writePair("name", item.name, 2, true);		
		fp.writePair("type", item.itemType, 2, true);
		fp.write("frames", 2);
		
		for(var i = 0; i <= 9999; i++) {
			var key = item.name + padNumber(i, 4);
			if(!(key in meta.frames)) {
				break;
			}
			var o = meta.frames[key].frame;
			fp.writePair("frame", o.x + ", " + o.y + ", " + o.w + ", " + o.h, 3);
		}
	});

	fp.close();

	timeline.currentFrame = originalFrame;
	timeline.currentLayer = originalLayer;
}


var debugTrace = true;
var leaveMetaDataOnDisk = true;

fl.outputPanel.clear();
var uri = fl.browseForFileURL("save", "Save file as", "Maki Document (*.mflas)", "mflas");
if(uri) {
	fl.trace("Export scene to: " + uri);
	exportScene(uri, debugTrace, leaveMetaDataOnDisk);
	fl.trace("Export complete");
} else {
	fl.trace("User cancelled");
}
