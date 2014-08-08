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
* $Id: slider_bar.js,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

		var BAR_WIDTH = 300;
		var SLIDER_WIDTH = 10;
		var currentSlider = null;
		var currentSliderMin = null;
		var currentSliderMax = null;

		function findPosX(obj)
		{
			var curleft = 0;
			if (typeof obj.offsetParent != 'undefined') {
				while (obj.offsetParent) {
					curleft += obj.offsetLeft
					obj = obj.offsetParent;
					if (obj.currentStyle && obj.currentStyle.width) {
						// IE forgets to take the border width into account when a width is set for the elt
						// so add it in ourselves
						if (!isNaN(borderWidth = parseInt(obj.currentStyle.borderLeftWidth))) {
							curleft += borderWidth;
						}
					}
				}
			} else if (obj.x) {
				curleft += obj.x;
			}
			if (document.body.currentStyle) {
				// IE needs the left margin to be added too
				curleft += parseInt(document.body.currentStyle.marginLeft);
			}
			return curleft;
		}

		function moveSlider(e)
		{
			if (currentSlider == null) return;
			var bar = currentSlider.parentNode;
			var barLeft = findPosX(bar, true);
			if (window.event) {
				var targetLeft = (window.event.clientX + document.body.scrollLeft);
			} else if (typeof e.pageX != "undefined") {
				var targetLeft = e.pageX;
			}
			if (targetLeft < barLeft) {
				targetLeft = barLeft;
			}
			if (targetLeft > barLeft + BAR_WIDTH - SLIDER_WIDTH) {
				targetLeft = barLeft + BAR_WIDTH - SLIDER_WIDTH;
			}
			currentSlider.getElementsByTagName('INPUT')[0].value = currentSliderMin + Math.round((currentSliderMax - currentSliderMin) * ((targetLeft - barLeft) / (BAR_WIDTH - SLIDER_WIDTH)));
			currentSlider.style.left = targetLeft + 'px';
		}

		var handleSliderMousedown = function(e)
		{
			currentSlider = this.getElementsByTagName('DIV')[0].getElementsByTagName('DIV')[0];
			var spans = this.parentNode.getElementsByTagName('SPAN');
			currentSliderMin = parseInt(spans[0].innerHTML.substr(2));
			currentSliderMax = parseInt(spans[1].innerHTML.substr(2));
			window.onmousemove = moveSlider;
			document.onmousemove = moveSlider;
			window.onmouseup = handleSliderMouseup;
			document.body.onmouseup = handleSliderMouseup;
			moveSlider(e);
		}

		function handleSliderMouseup()
		{
			window.onmousemove = null;
			document.onmousemove = null;
			currentSlider = null;
			window.onmouseup = null;
			document.body.onmouseup = null;
		}

		function sliderize(inputBox)
		{
			if (!document.getElementById) return;
			var descs = inputBox.parentNode.getElementsByTagName('SPAN');
			for (var i=0; i < descs.length; i++) {
				descs[i].style.display = 'none';
			}
			var sliderContainer = inputBox.parentNode;

			var newInputBox = document.createElement('INPUT');
			newInputBox.type = 'hidden';
			newInputBox.id = inputBox.id;
			newInputBox.name = inputBox.name;
			newInputBox.value = inputBox.value;

			var slider = document.createElement('DIV');
			slider.className = 'slider';
			slider.style.width = SLIDER_WIDTH + 'px';
			slider.appendChild(newInputBox);

			var sliderBar = document.createElement('DIV');
			sliderBar.className = 'slider-bar';
			sliderBar.style.width = BAR_WIDTH + 'px';
			sliderBar.appendChild(slider);

			var sliderBox = document.createElement('DIV');
			sliderBox.className = 'slider-box';
			if (!inputBox.disabled) {
				sliderBox.onmousedown = handleSliderMousedown;
			}
			sliderBox.appendChild(sliderBar);

			inputBox.parentNode.insertBefore(sliderBox, inputBox);
			inputBox.parentNode.removeChild(inputBox);
		}

		function initSliders()
		{
			var allDivs = document.getElementsByTagName('DIV');
			for (var j=0; j < allDivs.length; j++) {
				if (allDivs[j].className == 'slider') {
					initSlider(allDivs[j]);
				}
			}
		}

		function initSlider(slider)
		{
			var spans = slider.parentNode.parentNode.parentNode.getElementsByTagName('SPAN');
			var newInputBox = slider.parentNode.parentNode.parentNode.getElementsByTagName('INPUT')[0];
			var sliderMin = parseInt(spans[0].innerHTML.substr(2));
			var sliderMax = parseInt(spans[1].innerHTML.substr(2));
			slider.style.left = (findPosX(slider.parentNode) + Math.round((newInputBox.value / (sliderMax - sliderMin)) * BAR_WIDTH)) + 'px';
		}

		if (typeof preSliderOnLoad != 'function') {
			var preSliderOnLoad = window.onload;
			window.onload = function()
			{
				if (typeof preSliderOnLoad == 'function') preSliderOnLoad();
				var allDivs = document.getElementsByTagName('DIV');
				for (var j=0; j < allDivs.length; j++) {
					if (allDivs[j].className == 'slider-container') {
						sliderize(allDivs[j].getElementsByTagName('INPUT')[0]);
					}
				}
				initSliders();
			}
		}
