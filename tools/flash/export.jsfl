fl.outputPanel.clear();

var dom = fl.getDocumentDOM();
var timeline = dom.getTimeline();
var frameCount = timeline.frameCount;
var layerCount = timeline.layerCount;
var originalFrame = timeline.currentFrame;
var originalLayer = timeline.currentLayer;

fl.trace("frameCount " + frameCount);
fl.trace("layerCount " + layerCount);

var libraryItems = {};

function printBezier(depth, elem) {
	if(!elem || elem.@enabled != "1") {
		return;
	}
	s = "";
	for(var i = 0; i < depth; i++) {
		s += "\t";
	}
	var keyframes = elem.Keyframe;
	
	fl.trace(s+"property " + elem.@id);
	for(var i = 0; i < keyframes.length(); i++) {
		var kf = keyframes[i];
		fl.trace(s+"\tkey");
		fl.trace(s+"\t\tanchor " + kf.@anchor);
		fl.trace(s+"\t\ttimevalue " + kf.@timevalue);
		fl.trace(s+"\t\tprevious " + kf.@previous);
		fl.trace(s+"\t\tnext " + kf.@next);
	}
}

for(var li = 0; li < layerCount; li++) {
	// Have IDE select this layer
	timeline.currentLayer = li;
	var layer = timeline.layers[li];
	fl.trace("layer " + li);
	fl.trace("name \"" + layer.name + "\"");
	fl.trace("visible " + layer.visible);
	
	for(var fi = 0; fi < layer.frames.length;) {
		var frame = layer.frames[fi];
		if(fi != frame.startFrame) {
			continue;
		}
		
		// Have IDE select this frame
		timeline.currentFrame = fi;		
		fl.trace("\tkeyframe " + frame.startFrame + ", " + frame.duration);
		fl.trace("\ttweenType \"" + frame.tweenType + "\"");
		
		if(frame.isMotionObject() && frame.hasMotionPath()) {
			//frame.selectMotionPath(true);
			var xml = new XML(frame.getMotionObjectXML());
			fl.trace("\ttimeScale " + xml.@TimeScale);
			fl.trace("\tduration " + xml.@duration);
			fl.trace("\tinterp " + xml.TimeMap.@type);
			
			printBezier(1, xml..Property.(@id=="Motion_X"));
			printBezier(1, xml..Property.(@id=="Motion_Y"));
			printBezier(1, xml..Property.(@id=="Scale_X"));
			printBezier(1, xml..Property.(@id=="Scale_Y"));
		}
		
		frame.elements.forEach(function(e) {
			fl.trace("\t\telement");
			fl.trace("\t\t\telementType " + e.elementType);
			fl.trace("\t\t\tinstanceType " + e.instanceType);
			if(e.elementType == "instance") {
				if(e.instanceType == "bitmap") {
					fl.trace("\t\t\tpixelSize " + e.hPixels + ", " + e.vPixels);
				} else if(e.instanceType == "symbol") {
					fl.trace("\t\t\tsymbolType " + e.symbolType);
				} else {
					fl.trace("Unsupported instance type: " + e.instanceType);
					return;
				}
			} else {
				fl.trace("Unsupported element type: " + e.elementType);
				return;
			}
			
			fl.trace("\t\t\tname " + e.name);
			fl.trace("\t\t\tzindex " + e.depth);
			fl.trace("\t\t\tsize " + e.width + ", " + e.height);
			fl.trace("\t\t\tmatrix " + e.matrix.a + ", " + e.matrix.b + ", " + e.matrix.c + ", " + e.matrix.d + ", " + e.matrix.tx + ", " + e.matrix.ty);
			fl.trace("\t\t\ttransform " + e.transformX + ", " + e.transformY);
			fl.trace("\t\t\trotation " + e.rotation);			
			fl.trace("\t\t\tscale " + e.scaleX + ", " + e.scaleY);
			fl.trace("\t\t\tregistrationPoint " + e.x + ", " + e.y);			
			
			// Add this library item to the set of items that we must export
			libraryItems[e.libraryItem] = true;
		});
		
		fi += frame.duration;
	}
}

fl.trace("libraryItems");
for(var item in libraryItems) {
	fl.trace("\titem");
	fl.trace("\t\titemType " + item.itemType);
	fl.trace("\t\tname " + item.name);
	if(item instanceof SymbolItem) {
		fl.trace("\t\tsymbol");
		fl.trace("\t\tsymbolType " + item.symbolType);
	} else if(item instanceof BitmapItem) {
		fl.trace("\t\tbitmap");
	}
}

timeline.currentFrame = originalFrame;
timeline.currentLayer = originalLayer;
fl.trace("done");