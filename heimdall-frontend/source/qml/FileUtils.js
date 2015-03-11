function clipFileExtension(filename) {
	var periodIndex = filename.lastIndexOf('.');

	if (periodIndex > 0) {
		return filename.slice(0, periodIndex);
	} else if (periodIndex === 0) {
		return "";
	}

	return filename;
}

function filenameFromUrl(url) {
	var urlString = url.toString();
	return urlString.slice(urlString.lastIndexOf('/') + 1);
}

function filenameFromPath(path) {
	return filenameFromUrl(path);
}

function fileExtension(url) {
	var filename = filenameFromUrl(url);
	var periodIndex = filename.lastIndexOf('.');

	if (periodIndex >= 0) {
		return filename.slice(periodIndex + 1);
	}

	return "";
}

// TODO: Real implemention - call out to C++ and validate with QFileInfo etc.
function isFile(url) {
	var filename = filenameFromUrl(url);
	return filename.length > 0;
}

function isArchive(url) {
	var filename = filenameFromUrl(url);
	var extension = fileExtension(filename);
	return (extension === 'tar' || extension === 'zip')
		|| (extension === 'gz' && fileExtension(clipFileExtension(filename)) === 'tar');
}

function extractArchive(url) {

}
