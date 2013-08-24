
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
			this.buffer += indent + s + "\n";
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

function writeBezier(elem, fp, depth) {
	if(typeof(elem) == "undefined" || elem.@enabled != "1") {
		return;
	}
	fp.writePair("property", elem.@id.toLowerCase(), depth, true);
	var keyframes = elem.Keyframe;	
	for(var i = 0; i < keyframes.length(); i++) {
		var kf = keyframes[i];
		fp.write("key", depth+1);
		fp.writePair("anchor", kf.@anchor.toString().replace(",", ", "), depth+2);
		fp.writePair("time", kf.@timevalue, depth+2);
		fp.writePair("prev", kf.@previous.toString().replace(",", ", "), depth+2);
		fp.writePair("next", kf.@next.toString().replace(",", ", "), depth+2);
	}
}

function cleanName(name) {
	return name.replace(" ", "").toLowerCase();
}

function exportScene(uri, debugTrace) {
	var pathChunks = uri.split("/");
	var exportDir = pathChunks.slice(0, pathChunks.length-1).join("/");
	fl.trace(exportDir);
	
	var dom = fl.getDocumentDOM();
	var timeline = dom.getTimeline();
	var frameCount = timeline.frameCount;
	var layerCount = timeline.layerCount;

	var originalFrame = timeline.currentFrame;
	var originalLayer = timeline.currentLayer;

	var referencedLibraryItems = {};
	var libraryList = [];
	
	var fp = fopen(uri, debugTrace);
	fp.writePair("frame_count", frameCount);
	fp.writePair("layer_count", layerCount);
	fp.write("layers");

	for(var li = 0; li < layerCount; li++) {
		// Have IDE select this layer
		timeline.currentLayer = li;
		var layer = timeline.layers[li];
		
		fp.write("layer", 1);
		fp.writePair("name", layer.name, 2, true);
		fp.writePair("visible", layer.visible, 2);
		
		for(var fi = 0; fi < layer.frames.length;) {
			var frame = layer.frames[fi];
			if(fi != frame.startFrame) {
				continue;
			}
			
			// Have IDE select this frame
			timeline.currentFrame = fi;
			
			fp.write("keyframe", 2);
			fp.writePair("frame_start", frame.startFrame, 3);
			fp.writePair("frame_duration", frame.duration, 3);
			
			if(frame.isMotionObject() && frame.hasMotionPath()) {
				fp.write("tween", 3);
				fp.writePair("type", frame.tweenType, 4, true);
						
				var xml = new XML(frame.getMotionObjectXML());
				fp.writePair("time_scale", xml.@TimeScale, 4);
				fp.writePair("time_duration", xml.@duration, 4);
				fp.writePair("interpolation", xml.TimeMap.@type, 4, true);
				
				writeBezier(xml..Property.(@id=="Motion_X"), fp, 4);
				writeBezier(xml..Property.(@id=="Motion_Y"), fp, 4);
				writeBezier(xml..Property.(@id=="Scale_X"), fp, 4);
				writeBezier(xml..Property.(@id=="Scale_Y"), fp, 4);
			}
			
			fp.write("elements", 3);
			frame.elements.forEach(function(e) {
				fp.write("element", 4);
				fp.writePair("type", e.elementType, 5, true);
				fp.writePair("instance_type", e.instanceType, 5, true);
				
				if(e.elementType == "instance") {
					if(e.instanceType == "bitmap") {
						fp.writePair("pixel_size", e.hPixels + ", " + e.vPixels, 5);
					} else if(e.instanceType == "symbol") {
						fp.writePair("symbol_type", e.symbolType, 5, true);
					} else {
						fl.trace("Unsupported instance type: " + e.instanceType);
						return;
					}
				} else {
					fl.trace("Unsupported element type: " + e.elementType);
					return;
				}
				
				fp.writePair("name", e.name, 5, true);
				fp.writePair("z_index", e.depth, 5);
				fp.writePair("size", e.width + ", " + e.height, 5);
				fp.writePair("matrix", e.matrix.a + ", " + e.matrix.b + ", " + e.matrix.c + ", " + e.matrix.d + ", " + e.matrix.tx + ", " + e.matrix.ty, 5);
				fp.writePair("transform", e.transformX + ", " + e.transformY, 5);
				fp.writePair("rotation", e.rotation, 5);
				fp.writePair("scale", e.scaleX + ", " + e.scaleY, 5);
				fp.writePair("registration_point", e.x + ", " + e.y, 5);
								
				var index = 0;
				if(e.libraryItem.name in referencedLibraryItems) {
					index = referencedLibraryItems[e.libraryItem.name];
				} else {
					index = libraryList.length;
					referencedLibraryItems[e.libraryItem.name] = index;
					libraryList.push(e.libraryItem.name);
				}
				fp.writePair("library_index", index, 5);
			});
			
			fi += frame.duration;
		}
	}

	var lib = document.library;
	fp.write("library");
	libraryList.forEach(function(itemName) {
		var item = lib.items[lib.findItemIndex(itemName)];
		fp.write("item", 1);
		fp.writePair("name", cleanName(item.name), 2, true);		
		fp.writePair("type", item.itemType, 2, true);
		
		var uri = exportDir + "/" + cleanName(item.name);
		fp.writePair("path", FLfile.uriToPlatformPath(uri), 2, true);
		
		lib.selectItem(itemName, true, true);
		
		var sheet = new SpriteSheetExporter();
		sheet.layoutFormat = "JSON";
		sheet.addSymbol(item);
		
		var meta = sheet.exportSpriteSheet(uri, {format: "png", bitDepth:32, backgroundColor: "#00000000"});
		meta = eval("(" + meta + ")");
		
		var frames = [];
		for(var key in meta.frames) {
			frames.push(meta.frames[key])
		}
		fp.writePair("sheet_size", meta.meta.size.w + ", " + meta.meta.size.h, 2);
		fp.write("frames", 2);
		frames.forEach(function(f) {
			var o = f.frame;
			fp.writePair("frame", o.x + ", " + o.y + ", " + o.w + ", " + o.h, 3);
		});
	});
	
	fp.close();

	timeline.currentFrame = originalFrame;
	timeline.currentLayer = originalLayer;
}



fl.outputPanel.clear();
var uri = fl.browseForFileURL("save", "Save file as", "Maki Document (*.mdoc)", "mdoc");
if(uri) {
	fl.trace("Export scene to: " + uri);
	exportScene(uri, true);
	fl.trace("Export complete");
} else {
	fl.trace("User cancelled");
}
