/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: TSelect.js,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

/**
* Select class. Constructor
*
* @return null
* @access public
*/
TSelect = function(varname, select)
{
	this.select = document.getElementById(select);
	this.varname = varname;
	this.sel = null;
	this.input = null;
	this.results = Array();
	this.position = -1;
	this.curvalue = "";
	this.time = null;
	this.fromBegin = false;
	this.resname = "";
	this.searchfield = "text";
	this.valuefield = "value";


	/**
	* function fires when user press any key in the text field. It searches for words
	*
	* @param	word	value of the select
	* @param	evt		event object
	*
	* @return null
	* @access public
	*/
	this.search = function(word, evt)
	{
		var res = Array();
		var key = evt.keyCode;
		if (key == 13 || key == 27) {
			this.hide();
			evt.cancelBubble = true;
			evt.returnValue = false;
			this.process();
			return true;
		}
		if (key == 38 || key == 40) {
			this.position = this.position * 1 + 1 * ((key == 38)?-1:1);
			if (this.position < 0) {
				this.position = 0;
			}

			if (this.position > this.sel.options.length - 1) {
				this.position = this.sel.options.length - 1;
			}

			this.sel.selectedIndex = this.position;
			this.input.value = this.sel.options[this.position].text;
			this.input.style.fontWeight = "bold";

			if (this.select != null) {
				this.select.value = this.sel.value;
			}
			if (this.sel.style.display == "none") {
				this.sel.style.display = "block";
			}

			return true;
		}
		if (this.input == null) {
			this.refresh();
		}
		this.curvalue = this.input.value;
		if (word != "") {
			this.input.style.fontWeight = "normal";
			if (this.select != null && this.select.tagName == "SELECT") {
				var options = this.select.options;
			} else {
				var options = Clients;
				if (this.select == null) {
					this.select = document.getElementById(this.resname);
				}
			}
			for (var i = 0; i < options.length; i++) {
				if (options[i][this.searchfield].toUpperCase() == word.toUpperCase()) {
					this.input.style.fontWeight = "bold";
					this.input.value = options[i][this.searchfield];
					this.select.value = options[i][this.valuefield];
					this.hide();
					return true;
				}
				if ((this.fromBegin && options[i][this.searchfield].toUpperCase().indexOf(word.toUpperCase()) == 0) ||
					(!this.fromBegin && options[i][this.searchfield].toUpperCase().indexOf(word.toUpperCase()) > -1)) {
					var res2 = new Object();
					for (prop in options[i]) {
						res2[prop] = options[i][prop];
					}
					res.push(res2);
				}
			}
		}
		this.results = res;
		this.refresh();
	}


	/**
	* hides select box
	*
	* @param	val		to set or not value before hiding
	*
	* @return null
	* @access public
	*/
	this.hide = function(val)
	{
		if (typeof(val) != "undefined" && this.sel.selectedIndex > -1) {
			this.input.value = this.sel.options[this.sel.selectedIndex].text;
			this.input.style.fontWeight = "bold";
			if (this.select != null) {
				this.select.value = this.sel.value;
			}
		}
		this.sel.style.display = "none";
		this.position = -1;
		this.curvalue = "";
	}


	/**
	* shows select box and refresh position
	*
	* @return null
	* @access public
	*/
	this.refresh = function()
	{
		if (this.sel == null) {
			// positioning sel on the screen
			this.sel = document.getElementById(this.varname + "_select");
			this.input = document.getElementById(this.varname + "_input");
			var curtop = 0;
			var obj = this.input;
			if (obj.offsetParent)
			{
				while (obj.offsetParent)
				{
					curtop += obj.offsetTop;
					obj = obj.offsetParent;
				}
			}
			else if (obj.y) curtop += obj.y;
			var curleft = 0;
			obj = this.input;
			if (obj.offsetParent)
			{
				while (obj.offsetParent)
				{
					curleft += obj.offsetLeft;
					obj = obj.offsetParent;
				}
			}
			else if (obj.x) curleft += obj.x;
			this.sel.style.top = curtop + this.input.offsetHeight + "px";
			this.sel.style.left = curleft + "px";
			this.sel.style.width = this.input.offsetWidth + "px";
		}
		this.sel.options.length = 0;
		for (var i = 0; i < this.results.length; i++) {
			this.sel.options[i] = new Option("text", "value");
			this.build(this.sel.options[i], i);
		}
		this.position = -1;
		if (this.sel.options.length > 0) {
			this.sel.style.display = "block";
		} else {
			this.sel.style.display = "none";
		}
	}


	/**
	* build options for list
	* could be overwritten
	*
	* @param	option	option object
	* @oaram	num		number of the element in the "this.result" array
	*
	* @return null
	* @access public
	*/
	this.build = function(option, num)
	{
		option.text = this.results[num].text;
		option.value = this.results[num].value;
		option.style.background = this.results[num].style.background;

	}


	/**
	* build options for list
	* could be overwritten
	*
	* @return null
	* @access public
	*/
	this.process = function(){}


	/**
	* converts object to string
	*
	* @return null
	* @access public
	*/
	this.toString = function()
	{
		var res = "";
		var value = "";
		if (this.select != null && this.select.selectedIndex > -1) {
			value = this.select.options[this.select.selectedIndex].text;
		} else {
			res += '<input type="hidden" id="' + this.resname + '" name="' + this.resname + '" />';
		}
		res += '<input id="' + this.varname + '_input" type="text" class="selector" onkeyup="' + this.varname + '.search(this.value, event)" onblur="' + this.varname + '.time = setTimeout(\'' + this.varname + '.hide(1)\', 500)" onfocus="clearTimeout(' + this.varname + '.time)" value="' + value + '" />\n';
		res += '<select multiple="multiple" onfocus="clearTimeout(' + this.varname + '.time)" onchange="' + this.varname + '.hide(1)" id="' + this.varname + '_select" class="results" onblur="' + this.varname + '.time = setTimeout(\'' + this.varname + '.hide(1)\', 500)"></select>\n';
		return res;
	}


	//initialisation

	if (this.select != null) {
		this.select.style.display = "none";
	} else {
		this.resname = select;
	}
}

