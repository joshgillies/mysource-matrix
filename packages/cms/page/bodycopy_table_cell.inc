<?php


class BodyCopy_Table_Cell extends Resolve_Object {

	var $attributes = Array();  # an array of attributes for the TD tag

	 ##############################
	# Constructor, num_cols for 
	function BodyCopy_Table_Cell() {

	}#end constructor

	 #########################################
	# set an attribute for the table tag
	function set_attribute($name, $value) {
		$name = strtolower($name);
		if (!isset($this->attributes[$name])) $this->attributes[$name] = '';
		if ($this->attributes[$name] == $value) return false;
		$this->attributes[$name] = $value;
		return true;
	}

	 ##############################
	# Delete this bastard
	function delete() {

		unset($this->attributes);

		$this = new BodyCopy_Table_Cell();
		return true;

	}#end delete()


	 ###########################################
	# how many columns does this cell span ??
	function colspan() {
		if (empty($this->attributes['colspan'])) return 1;
		return ((int) $this->attributes['colspan'] > 0) ? (int) $this->attributes['colspan'] : 1;
	}

	 ###########################################
	# and how many rows does this cell span ??
	function rowspan() {
		if (empty($this->attributes['rowspan'])) return 1;
		return ((int) $this->attributes['rowspan'] > 0) ? (int) $this->attributes['rowspan'] : 1;
	}


	function paint() {
		echo '<td';
		foreach($this->attributes as $name => $val) {
			if ($val == '') continue;
			if (strtolower($name) == 'nowrap') {
				if ($val) echo ' nowrap';
			} else {
				echo ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}
		echo '>';
		echo 'cell contents';
		echo '</td>';
	}


	 #####################################################
	# prints the backend interface for this table cell
	function paintCell($prefix, $tableid, $rowid, $cellid) {
		global $_BODYCOPY;

		$attribute_list = '';
		$colspaning = 0;
		foreach($this->attributes as $name => $val) {
			if ($val == '') continue;
			if (strtolower($name) == 'nowrap') {
				if ($val) $attribute_list .= ' nowrap';
			} else {
				if (strtolower($name) == 'colspan') {
					$colspaning = $val;
				}
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}

		// print out the edit buttons
		?> 
		<td class="bodycopy-edit" <?php echo ($colspaning > 0) ? 'colspan="'.$colspaning.'"' : '';?> id="<?php echo $prefix.'_cell_'.$tableid.'_'.$rowid.'_'.$cellid?>" align="right">
			<?php
			print_bodycopy_icon("Javascript: bodycopy_edit_table_cell_properties('$prefix', $tableid, $rowid, $cellid);", 'Edit Cell '.($cellid + 1).'\'s Properties', (($attribute_list) ? 'Current Properties :<br>'.$attribute_list : ''), 'edit_cell_properties');
			?>
		</td>
		<?php
	}#end print_backend()


	 ###############################################
	# process the input created by print_backend()
	function processCell($prefix, $tableid, $rowid, $cellid) {
		global $bodycopy_action, $bodycopy_name, $bodycopy_data, $bodycopy_saved;
		global $_BODYCOPY;
		
		$updated = false;
		if (isset($bodycopy_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'])) {
			foreach($bodycopy_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'] as $name => $value) {
				if ($this->set_attribute($name, $value)) $updated = true;
			}
		}
		return $updated;
	}//end processCell()


	  ###########################################################
	 # prints the JavaScript needed by the backend functions for 
	# this table 
	function print_backend_js_data($prefix, $tableid, $rowid, $cellid) {
		?> 
									case <?php echo $cellid?> :
										retVal["attributes"] = '<?php echo var_serialise($this->attributes, true)?>';
									break;
		<?php
	}//end print_backend_js_data()


}//end Class BodyCopy_Table_Cell

?>
