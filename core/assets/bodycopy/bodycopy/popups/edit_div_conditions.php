<?php
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
*
*/


/**
* DIV Properties Pop-Up
*
* Purpose
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s',time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script type="text/javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script type="text/javascript">

	function popup_init() {
		var data = owner.bodycopy_current_edit["data"]["attributes"]["conditions"];
		if(data === undefined || data === null) return;
		
		var f = document.main_form;
		f.condition_rules_status.value = (data['condition_rules_status'] == null) ? "enable" : data['condition_rules_status'];
		f.logical_op_groups.value  	   = (data['logical_op_groups']       == null) ? "all_match" : data['logical_op_groups'];
		
		if(data['conditions'] !== null) {
		    for(var i = 0; i < data['conditions'].length;  i++) {
			var group = data['conditions'][i];
			add_group(group);
		    }
		}	
	}// end popup_init()

	function popup_save(f) {
		var data =  owner.bodycopy_current_edit["data"]["attributes"];
		var condition_data = new Object();
		condition_data["condition_rules_status"]       = owner.form_element_value(f.condition_rules_status);
		condition_data["logical_op_groups"]             = owner.form_element_value(f.logical_op_groups);
		var conditions = [];
		var groups = document.querySelectorAll('.condition_group');
		for (var i =0; i < groups.length; i++) {
		    var condition = {};
		    condition['logical_op'] = groups[i].querySelector('.logical_op_conditions').value;
		    var all_conditions = groups[i].querySelectorAll('.selected_condition');
		    condition['conditions'] = [];
		    for(var j=0; j< all_conditions.length; j++) {
			condition['conditions'].push(all_conditions[j].value);
		    }
		    conditions.push(condition);
		}
		condition_data['conditions'] = conditions;
		data['conditions'] = condition_data;

		owner.bodycopy_save_div_properties(data);
	}
	
	function set_class(value) {
		document.main_form.css_class.value = value;
	}
	
	// add a condition group to the table
	function add_group(data) {
	    
	    // add group row
	    var table = document.getElementById('condition_groups_table');
	    var tr = table.insertRow(-1);
	    var rowIndex = tr.rowIndex;
	    var td = tr.insertCell(0);
	    var fieldset = document.createElement('fieldset');
	    var h2 = document.createElement('h2');
	    var b = document.createElement('b');
	    var name = document.createTextNode('Condition Group');
	    b.appendChild(name);
	    h2.appendChild(b);
	    td.appendChild(h2);
	    td.appendChild(fieldset);
	    
	    // add table for conditions
	    var condition_table = document.createElement('table');
	    condition_table.setAttribute('class', 'condition_group');
	    var tbdy=document.createElement('tbody');
	    condition_table.style.width='100%';
	    condition_table.appendChild(tbdy);
	    fieldset.appendChild(condition_table);
	    	    
	    // add "add condition" link
	    var div =  document.createElement('div');
	    div.style = 'text-align: center;';
	    var add_link = document.createElement('a');
	    add_link.href = '#';
	    add_link.innerHTML = '<img src="<?php echo sq_web_path('lib')?>/web/images/icons/add.png" alt="Add" title="Add condition group " class="sq-icon sq-link-icon small">';
	    add_link.appendChild(document.createTextNode('Add condition'));
	    add_link.onclick = function () {
		add_condition(tbdy, null);
	    };
	    div.appendChild(add_link);
	    fieldset.appendChild(div);
	    
	    // add the logical grouping option
	    var tr =document.createElement('tr');
	    var td_label = document.createElement('td');
	    var td_input = document.createElement('td');
	    td_label.class = 'label';
	    td_label.appendChild(document.createTextNode('Logical grouping:'));
	    tr.appendChild(td_label);
	    tr.appendChild(td_input);
	    

	    var select = document.createElement("select");
	    select.setAttribute("class", "logical_op_conditions");
	    var option_1 = document.createElement("option");
	    option_1.setAttribute('value', 'all_match');
	    option_1.innerHTML = 'All conditions must match';
	    select.appendChild(option_1);
	    var option_2 = document.createElement("option");
	    option_2.setAttribute('value', 'one_match');
	    option_2.innerHTML = 'At least 1 condition must match';
	    select.appendChild(option_2);

	    // if there is pre-set logical op, set it
	    if(data !== null && typeof data.logical_op != 'undefined') {
		if(data.logical_op === 'all_match') {
		    option_1.setAttribute('selected', 'selected');
		}
		else {
		    option_2.setAttribute('selected', 'selected');
		}
	    }

	    td_input.appendChild(select);
	    tbdy.appendChild(tr);

	    // if there is pre-set conditions, set it
	    if(data !== null && typeof data.conditions != 'undefined') {
		var available_conditions =owner.bodycopy_current_edit["available_conditions"];
		var number = 0;
		for (var i=0; i< data.conditions.length; i++) {
		    for (var j=0; j<available_conditions.length; j++) {
			// just to make sure we only set conditions that is in available conditions
			if(data.conditions[i] === available_conditions[j]) {
			    add_condition(tbdy, data.conditions[i]);
			    number++;
			}
		    }
		}
	    }
	    else {
		 // add the initial condition
		add_condition(tbdy, null);
	    }

	    
	    // print delete group icon
	    var deleteIcon=document.createElement('img');
	    deleteIcon.src = "<?php echo(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png'); ?>";
	    deleteIcon.alt = 'Delete this group';
	    deleteIcon.title = 'Delete this group';
	    deleteIcon.className = 'sq-popup-btn';
	    deleteIcon.onclick = function () {
		this.parentNode.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode.parentNode);
		return false;
	    };
	    h2.insertBefore(deleteIcon, b);

	    return false;
	}
	
	
	function add_condition(parent, content) {
	    var tr =document.createElement('tr');
	    var td_label = document.createElement('td');
	    var td_input = document.createElement('td');
	    td_label.className = 'label';
	    td_input.className = 'condition';
	    td_label.appendChild(document.createTextNode('Condition:'));
	    tr.appendChild(td_label);
	    tr.appendChild(td_input);
	    parent.appendChild(tr);
	    
	    // add condition dropdown
	    var conditions =owner.bodycopy_current_edit["available_conditions"];
	    var select = document.createElement("select");
	    select.setAttribute("class", "selected_condition");
	    
	    for (var i = 0; i < conditions.length; i++) {
		var option = document.createElement("option");
		option.setAttribute('value', conditions[i]);
		if(content !== null && content === conditions[i]) {
		    option.setAttribute('selected', 'selected');
		}
		var condition_title_array = conditions[i].split(':');
		var parentid = condition_title_array.pop(); 
		var condition_title_part = condition_title_array.pop();
		var condition_title_string = condition_title_part.split('_').join(' ');
		option.innerHTML = condition_title_string + ' (#' + parentid +  ')';
		select.appendChild(option);
	    }
	    td_input.appendChild(select);

	    // print delete condition icon
	    var deleteIcon=document.createElement('img');
	    deleteIcon.src = "<?php echo(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png'); ?>";
	    deleteIcon.className = 'sq-popup-btn small';
	    deleteIcon.alt = 'Delete this condition';
	    deleteIcon.title = 'Delete this condition';
	    deleteIcon.style = 'cursor: pointer;';
	    deleteIcon.onclick = function () {
		this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);
		return false;
	    };
	    td_input.appendChild(deleteIcon);

	    return false;
	}

</script>

<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('cancel');?>" class="sq-icon">
	</a>
	DIV Condition Rules
</h1>
<form id="main_form" name="main_form"  style="height:410px;overflow: auto;">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table width="100%" border="0" >
	<tr>
		<td colspan="2">
		<h2>Condition settings</h2>
		<fieldset>
			<table style="width:100%">
				<tr>
					<td class="label">Condition rules:</td>
					<td>
					    <select name="condition_rules_status">
							<option value="enable" selected ><?php echo translate('enable'); ?></option>
							<option value="disable"><?php echo translate('disable'); ?></option>
						</select>
					</td>
				</tr>
				<tr>
					<td class="label">Logical grouping:</td>
					<td>
					    <select name="logical_op_groups" id="logical_op_groups">
							<option value="all_match"selected >All groups must match</option>
							<option value="one_match">At least 1 group must match</option>
						</select>
					</td>
				</tr>
			</table>
		</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
		    <table style="width:100%" id="condition_groups_table" class="sq-conditions-group-table">				
		    </table>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2 style="text-align: center; border-bottom: none;">
			    <a href="#" onClick="add_group(null); return false;" class="sq-full-width-btn sq-add-condition-group-link">
			    	<img src="<?php echo sq_web_path('lib')?>/web/images/icons/add.png" alt="Add" title="Add condition group " class="sq-icon sq-link-icon">Add condition group
			    </a>
			</h2>
		</td>
	</tr>
	<tr class="sq-popup-footer">
		<td align="left">
			<input type="button" class="" name="cancel" onClick="javascript: popup_close(); return false;" value="<?php echo translate('cancel'); ?>"/>
		</td>
		<td align="right">
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form); return false;" value="<?php echo translate('save'); ?>"/>
		</td>
	</tr>
</table>
</form>
<?php include(dirname(__FILE__).'/footer.php'); ?>
