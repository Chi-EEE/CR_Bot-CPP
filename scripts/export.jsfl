// Made by Chi-EEE
// docs @ https://github.com/AdobeDocs/developers-animatesdk-docs

function startsWith(str, word) {
    return str.lastIndexOf(word, 0) === 0;
}

var debug = false;

if (!debug) {
	fl.addEventListener("documentOpened", openDocument);
} else {
	openDocument();
}

function openDocument() {
	var dom = fl.getDocumentDOM();
	var domName = dom.name.substring(0, dom.name.length - 4);

	var outputFolder = 'file:///C|/Users/admin/Downloads/SC2FLATest/output'
	// var outputFolder = fl.browseForFolderURL("Select the folder to export PNG sequences");

    fl.trace("Opening document: " + dom.name);

	// Get the document's library
	var library = dom.library;
    // fl.trace(outputFolder);

	var exportItems = new Array();
	// Loop through all items in the library and print them
	for (var i = 0; i < library.items.length; i++) {
		var libItem = library.items[i];
		// fl.trace("libItem.name: " + libItem.name);
		if (startsWith(libItem.name, "exports/") && libItem.itemType == "movie clip") {
			exportItems.push(libItem);
		}
	}
	for (var count = 0; count < 10; count++) {
		for (var i = 0; i < exportItems.length; i++) {
			var exportItem = exportItems[i];
			var exportName = exportItem.name.substring(8);
			var exportFolder = outputFolder + "/" + domName + "/" + exportName;
			var frames = FLfile.listFolder(exportFolder + "/" + "*.png");
			var frameCount = 0;
			if (exportItem.timeline.layerCount > 0) {
				frameCount = exportItem.timeline.layers[0].frameCount
			}
			if (frames.length != frameCount) {
				// fl.trace("exportName: " + exportName);
				// fl.trace("frames.length: " + frames.length);
				// fl.trace("frameCount: " + frameCount);
				FLfile.createFolder(exportFolder);
				exportItem.exportToPNGSequence(exportFolder + "/" + "frame-.png");
				if (frames.length != frameCount) {
					var newFrames = FLfile.listFolder(exportFolder + "/" + "*.png");
					if (newFrames.length == frameCount) {
						exportItems.splice(i, 1);
					}
				}
			}
		}
	}
	fl.closeDocument(dom, false);
}