var MatrixWorkflowSchema = {
	
	currentDiv: null,
	stepDivToMove: null,
	startDivPos: null,
	startMousePos: null,

	isIE: function()
	{
		return navigator.userAgent.indexOf("MSIE") != -1;
	},

	// Calculate the position of an element
	// x, y = top-left corner position
	// cx, cy = width and height
	elPosition: function(el)
	{
		var el_x = 0;
		var el_y = 0;
		var el_cx = el.offsetWidth;
		var el_cy = el.offsetHeight;

		while (el != null) {
			el_x += el.offsetLeft;
			el_y += el.offsetTop;
			el = el.offsetParent;
		}
	
		return {x: el_x, y: el_y, cx: el_cx, cy: el_cy};
	},

	// current mouse position relative to the page
	mousePosition: function(event)
	{
		var mouse_x = 0;
		var mouse_y = 0;

		if (event.pageX) {
			mouse_x = event.pageX;
			mouse_y = event.pageY;
		} else if (event.clientX) {
			mouse_x = event.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
			mouse_y = event.clientY + document.body.scrollTop  + document.documentElement.scrollTop;
		}

		return {x: mouse_x, y: mouse_y};
	},


	activateDropPoints: function(step_div)
	{
		var self = this;
		step_div.validDrops = [];
		dividers = this.getElementsByClassName(step_div.parentNode, 'sq-workflow-step-divider');

		// All workflow steps have a divider before them
		previous_divider = step_div.previousSibling;

		// Workflow steps have either a divider, or an escalation DIV, so skip until we get one of the former
		next_divider = step_div.nextSibling;
		while (next_divider.className != 'sq-workflow-step-divider') {
			next_divider = next_divider.nextSibling;
		}

		for (var x = 0; x < dividers.length; x++) {
			divider_div = dividers[x];
			if ((divider_div.parentNode == step_div.parentNode) && (divider_div != previous_divider) && (divider_div != next_divider)) {
				step_div.validDrops[step_div.validDrops.length] = divider_div;
				divider_div.style.visibility = 'visible';
			}
		}

	},

	moveBeforeDivider: function(step_div, divider)
	{
		var parent_div = step_div.parentNode;
		
		// First check to see if we have to bring an escalation step with us
		prev_div = step_div.previousSibling;
		next_div = step_div.nextSibling;

		parent_div.removeChild(step_div);
		parent_div.insertBefore(step_div, divider);
		
		if (next_div && (next_div.className == 'sq-workflow-step-escalation-indent')) {
			parent_div.removeChild(next_div);
			parent_div.insertBefore(next_div, divider);
		}

		if (prev_div && (prev_div.className == 'sq-workflow-step-divider')) {
			parent_div.removeChild(prev_div);
			parent_div.insertBefore(prev_div, step_div);
		} else {
			new_divider = document.createElement('div');
			new_divider.className = 'sq-workflow-step-divider';
			parent_div.insertBefore(new_divider, step_div);
		}
	},

	makeDraggable: function(step_div)
	{
		var drag_div = this.getElementsByClassName(step_div, 'sq-workflow-step-drag-handle');
		var self = this;

		drag_div = drag_div[0];

		if (drag_div.parentNode.parentNode.validDrops.length == 0) {
			drag_div.style.visibility = 'hidden';
		}

		drag_div.onmousedown = function(event) {
			var event = event ? event : window.event;
			
			if (event.button == (self.isIE() ? 1 : 0)) {
				var step_div = drag_div.parentNode.parentNode;
				var parent_div = step_div.parentNode;

				if (!self.currentDiv && (step_div.validDrops.length > 0)) {
					self.stepDivToMove = step_div;
							
					for (var x = 0; x < self.stepDivToMove.validDrops.length; x++) {
						self.stepDivToMove.validDrops[x].backgroundColor = '#eee';
					}
					var dimensions = self.elPosition(step_div);
					self.currentDiv = document.createElement('div');
					self.currentDiv.className = 'sq-workflow-step';
					self.currentDiv.innerHTML    = step_div.innerHTML;
					document.body.insertBefore(self.currentDiv, null);

					self.currentDiv.style.position = 'absolute';
					self.currentDiv.style.left   = dimensions.x + 'px';
					self.currentDiv.style.top    = dimensions.y + 'px';
					self.currentDiv.style.width  = dimensions.cx + 'px';
					self.currentDiv.style.height = dimensions.cy + 'px';
					self.currentDiv.style.opacity = 0.5;
					self.currentDiv.style.filter = 'alpha(opacity=50)';
					self.currentDiv.style.zIndex = 999;

					self.startMousePos = self.mousePosition(event);
					self.startDivPos   = self.elPosition(self.currentDiv);

					// For some reason IE places the event on the document rather than
					// the window.
					topEl = self.isIE() ? document : window;

					topEl.onmousemove = function(event) {
						var event = event ? event : window.event;

						if (self.currentDiv) {
							//event.preventDefault();
							var mousePos = self.mousePosition(event);
							self.currentDiv.style.top = self.startDivPos.y + mousePos.y - self.startMousePos.y;

							for (var x = 0; x < self.stepDivToMove.validDrops.length; x++) {
								var divider_div = self.stepDivToMove.validDrops[x];
								var dividerDim = self.elPosition(divider_div);
								if ((mousePos.y >= dividerDim.y) && (mousePos.y < dividerDim.y + dividerDim.cy)) {
									// here
									divider_div.style.backgroundColor = '#999';
								} else {
									divider_div.style.backgroundColor = '#eee';
								}
							}
						}

						return false;
					};

					topEl.onmouseup = function(event) {
						var event = event ? event : window.event;

						this.onmousemove = null;
						if (self.currentDiv) {
							var mousePos = self.mousePosition(event);
							var foundDivider = null;
							for (var x = 0; x < self.stepDivToMove.validDrops.length; x++) {
								var divider_div = self.stepDivToMove.validDrops[x];
								divider_div.style.backgroundColor = 'transparent';
								var dividerDim = self.elPosition(divider_div);
								if ((mousePos.y >= dividerDim.y) && (mousePos.y < dividerDim.y + dividerDim.cy)) {
									// here
									foundDivider = divider_div;
								}
							}

							if (foundDivider) {
								self.moveBeforeDivider(self.stepDivToMove, foundDivider);
							}

							document.body.removeChild(self.currentDiv);
							self.currentDiv = null;	
							self.stepDivToMove = null;
					
							// realign the drop points
							workflow_dividers = self.getElementsByClassName(document, 'sq-workflow-step-divider');
							for (var x = 0; x < workflow_dividers.length; x++) {
								workflow_dividers[x].style.visibility = 'hidden';
							}

							workflow_steps = self.getElementsByClassName(document, 'sq-workflow-step');
							for (var x = 0; x < workflow_steps.length; x++) {
								var workflow_step = workflow_steps[x];
								self.activateDropPoints(workflow_step);
							}

						}

						return false;
					};

				}//end if (self.currentDiv == null)
			}//end if (event.button == 0)
			
			return false;
		};

	},

	onLoad: function()
	{
		workflow_steps = this.getElementsByClassName(document, 'sq-workflow-step');
		for (var x = 0; x < workflow_steps.length; x++) {
			var workflow_step = workflow_steps[x];
			this.activateDropPoints(workflow_step);
			this.makeDraggable(workflow_step);
		}
	},

	// Degrade gracefully for new getElementsByClassName function

	getElementsByClassName: function(element, className)
	{
		if (element.getElementsByClassName) {
			return element.getElementsByClassName(className);
		} else if (document.evaluate) {
			// If XPath is available (such as in Firefox 2), try that first
			result = document.evaluate('.//*[contains(concat(" ", @class, " "), " ' + className + ' ")]', element, null, XPathResult.ORDERED_NODE_ITERATOR_TYPE, null);
			result_arr = [];
			while (node = result.iterateNext()) {
				result_arr[result_arr.length] = node;
			}
			return result_arr;
		} else {
			allElements = element.getElementsByTagName('*');
			wantedElements = [];

			for (var x = 0; x < allElements.length; x++) {
				classes = allElements[x].className.split(' ');
				for (var y = 0; y < classes.length; y++) {
					if (classes[y] == className) {
						wantedElements[wantedElements.length] = allElements[x];
						break;
					}
				}//end for (y in classes)
			}//end for (x in allElements)

			return wantedElements;
		}
	}

};


