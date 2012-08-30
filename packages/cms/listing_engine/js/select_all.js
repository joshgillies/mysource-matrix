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
* $Id: select_all.js,v 1.8 2012/08/30 00:58:08 ewang Exp $
*
*/
var select_list;
function init_select_list_array() {
	if (!select_list) {
		select_list = new Array();
	}
}

function sq_listing_check_state(el, prefix) {
	var i, j, k;

	form = el.form;
	assetPrefix = form.id;

	// do tests to work out which check boxes should be checked/unchecked along
	// with the aggregate that we are unchecking

	to_check = [el.name];
	tested = [];
	tested[el.name] = el.checked;

	while(to_check.length > 0) {
		this_el = to_check.shift();

		for (i in select_list[assetPrefix]) {
			lhs = select_list[assetPrefix][i][0];
			rhs = select_list[assetPrefix][i][1];

			for(j in lhs) {
				full_name = prefix + lhs[j];
				if (full_name == this_el) {
					// a match - add it to what we have to modify,
					// and recurse down
					for(k in rhs) {
						full_name = prefix + rhs[k];
						tested[full_name] = el.checked;
						to_check.push(full_name);
					}
					break;
				}
			}
		}
	}

	// now check/uncheck what check boxes actually exist

	for (i in tested) {
		if (el = form.elements[i]) {
			if (el.length) {
				for (j = 0; j < el.length; j++) {
					el[j].checked = tested[i];
				}
			} else {
				el.checked = tested[i];
			}
		}
	}

	// now do our tests as to what aggregate check boxes should or shouldn't
	// be checked - we don't care whether the aggregate checkboxes are set or
	// not, because they are important anyway

	tested = [];

	for (i in select_list[assetPrefix]) {
		lhs = select_list[assetPrefix][i][0];
		rhs = select_list[assetPrefix][i][1];
		select_all = true;
		for(j in rhs) {
			full_name = prefix + rhs[j];

			// have we tested this already?
			if (typeof tested[full_name] == 'undefined') {
				if (el = form.elements[full_name]) {
					if (el.length) {
						tested[full_name] = true;
						for (k = 0; k < el.length; k++) {
							tested[full_name] = tested[full_name] && el[k].checked;
						}
					} else {
						tested[full_name] = el.checked;
					}
				}
			}

			// do we have a value now?
			if (tested[full_name] != 'undefined') {
				select_all = select_all && tested[full_name];
			}
		}

		// set the aggregate to what it should be
		for(j in lhs) {
			full_name = prefix + lhs[j];
			tested[full_name] = select_all;
		}
	}

	// now check/uncheck what actually exists

	for (i in tested) {
		if (el = form.elements[i]) {
			if (el.length) {
				for (j = 0; j < el.length; j++) {
					el[j].checked = tested[i];
				}
			} else {
				el.checked = tested[i];
			}
		}
	}

}//end sq_listing_check_state()
