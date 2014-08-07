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
* $Id: sort_order_manip.js,v 1.4 2012/08/30 01:09:08 ewang Exp $
*
*/



/**
* Moves a question in the question ordering
*
* @param object	form			the form OBJECT containing the question
* @param string	prefix			the form element prefix
* @param int	currentOrder	the question we are moving
* @param int	nextOrder		the question we are switching with - if moving:
*									- DOWN this would be currentOrder + 1
*									- UP this would be currentOrder - 1
*
* @return void
* @access public
*/
function moveQuestion(form, prefix, currentOrder, nextOrder) {

	if (!document.getElementById) {
		alert(js_translate('cms_simple_form_cannot_reorder'));
		return;
	}

	// the 'indices' of the elements we need to flip
	// Explanation of div names:
	// 't' = question type, 'o' = question name, 'a' = assetid, 'so' = sort order
	elements = ['t', 'o', 'so'];

	for(index in elements) {
		el = elements[index];
		var currentElement = document.getElementById(prefix + '_order_' + el + currentOrder);
		var nextElement = document.getElementById(prefix + '_order_' + el + nextOrder);

		// if there is no 'next' element, then this is the last one; we shouldn't be running
		if (!nextElement) {
			return;
		}

		// can we do this
		if (!currentElement.innerHTML) {
			alert(js_translate('cms_simple_form_cannot_reorder'));
			return;
		}

		// now swap them
		var temp = currentElement.innerHTML;
		currentElement.innerHTML = nextElement.innerHTML;
		nextElement.innerHTML = temp;
	}

	// mark these rows as dirty by changing their colour
	// Explanation of div names: 'so' = original sort order,
	// 'soc' = 'changed to' sort order, 'soa' = sort order arrow
	orders = [currentOrder, nextOrder];
	for(index in orders) {
		oldValue = parseInt(document.getElementById(prefix + '_order_so' + orders[index]).innerHTML);
		newValue = parseInt(document.getElementById(prefix + '_order_soc' + orders[index]).innerHTML);

		if (oldValue == newValue) {
			document.getElementById(prefix + '_order_soc' + orders[index]).style.visibility = 'hidden';
			document.getElementById(prefix + '_order_soa' + orders[index]).style.visibility = 'hidden';
			document.getElementById(prefix + '_order_so' + orders[index]).style.textDecoration = 'none';
			document.getElementById(prefix + '_order_row' + orders[index]).className = '';
		} else {
			document.getElementById(prefix + '_order_soa' + orders[index]).style.visibility = 'visible';
			document.getElementById(prefix + '_order_soc' + orders[index]).style.visibility = 'visible';
			document.getElementById(prefix + '_order_so' + orders[index]).style.textDecoration = 'line-through';
			document.getElementById(prefix + '_order_row' + orders[index]).className = 'alt';

			document.getElementById(prefix + '_order_soa' + orders[index]).innerHTML = '';

			if (oldValue > newValue) {
				moved_up = document.getElementById(prefix + '_moved_up').cloneNode(true);
				moved_up.style.display = 'inline';
				document.getElementById(prefix + '_order_soa' + orders[index]).appendChild(moved_up);
			} else {
				moved_down = document.getElementById(prefix + '_moved_down').cloneNode(true);
				moved_down.style.display = 'inline';
				document.getElementById(prefix + '_order_soa' + orders[index]).appendChild(moved_down);
			}
		}
	}

	// switch the 'checked for deletion' parameters
	temp = form.elements[prefix + '_order[delete][' + nextOrder + ']'].checked;
	form.elements[prefix + '_order[delete][' + nextOrder + ']'].checked = form.elements[prefix + '_order[delete][' + currentOrder + ']'].checked;
	form.elements[prefix + '_order[delete][' + currentOrder + ']'].checked = temp;

	// switch the reorder values
	temp = form.elements[prefix + '_order[reorder][' + nextOrder + ']'].value;
	form.elements[prefix + '_order[reorder][' + nextOrder + ']'].value = form.elements[prefix + '_order[reorder][' + currentOrder + ']'].value;
	form.elements[prefix + '_order[reorder][' + currentOrder + ']'].value = temp;

}//end moveQuestion()


// Replacements for option list attribute functions of similar name

var expandListFormFn = new Function('expandOptionListForm(this)');
var deleteRowFormFn = new Function('deleteOptionListRowForm(this); return false;');

/**
* Expands the question list to offer more questions
*
* @param object	input			the input OBJECT that we are editing now
*
* @return void
* @access public
*/
function expandOptionListForm(input)
{
	question_list = input.parentNode.parentNode;

	i = 0;
	filled = false;

	divs = question_list.getElementsByTagName('DIV');

	if (divs.length > 1) {
		div_node = divs[divs.length - 2];
		input_node = div_node.firstChild;
		select_node = input_node.nextSibling;

		if ((input_node.value != '') && (select_node.value != '')) {
			filled = true;
		}
	}

	if (!filled) {
		div_node = divs[divs.length - 1];
		input_node = div_node.firstChild;
		select_node = input_node.nextSibling;

		if ((input_node.value != '') || (select_node.value != '')) {
			filled = true;
		}
	}

	// add the extra fields
	var currentDiv = input.parentNode;

	if (filled) {
		var newDiv = currentDiv.cloneNode(true);
		newDiv.firstChild.value = '';
		newDiv.firstChild.nextSibling.value = '';
		newDiv.firstChild.onfocus = expandListFormFn;
		newDiv.firstChild.nextSibling.onfocus = expandListFormFn;
		newDiv.firstChild.nextSibling.nextSibling.onclick = deleteRowFormFn;
		currentDiv.parentNode.appendChild(newDiv);
	}

}//end expandOptionListForm()


/**
* Deletes a question off the question list
*
* @param object	button			the input OBJECT that we are editing now
*
* @return void
* @access public
*/
function deleteOptionListRowForm(button)
{
	var div = button.parentNode;
	div.parentNode.removeChild(div);

}//end deleteOptionListRowForm()
