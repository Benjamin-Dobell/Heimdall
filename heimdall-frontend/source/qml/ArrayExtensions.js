Array.prototype.extend = function(other) {
	other.forEach(function(item) { this.push(item) }, this);
};
