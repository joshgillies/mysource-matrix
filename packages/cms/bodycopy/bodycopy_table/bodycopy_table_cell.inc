<?php

/**
* Bodycopy_Table_Cell
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class BodyCopy_Table_Cell extends Resolve_Object {

	var $attributes = Array(); // an array of attributes for the TD tag

	/**
	* Constructor
	*
	*/
	function BodyCopy_Table_Cell()
	{
		// nothing to see here
	}


	/**
	* Sets an attribute value in the attributes var
	* Internal checking of these attributes goes here
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function set_attribute($name, $value)
	{
		$name = strtolower($name);
		if (!isset($this->attributes[$name])) $this->attributes[$name] = '';
		if ($this->attributes[$name] == $value) return false;
		$this->attributes[$name] = $value;
		return true;
	}//end set_attribute()


	/**
	* Delete this bastard
	*
	* @return boolean
	* @access public
	*/
	function delete()
	{
		unset($this->attributes);
		$this = new BodyCopy_Table_Cell();
		return true;

	}//end delete()


	/**
	* How many columns does this cell span ??
	*
	* @return integer
	* @access public
	*/
	function colspan()
	{
		if (empty($this->attributes['colspan'])) return 1;
		return ((int) $this->attributes['colspan'] > 0) ? (int) $this->attributes['colspan'] : 1;
	}//end colspan()


	/**
	* How many rows does this cell span ??
	*
	* @return integer
	* @access public
	*/
	function rowspan()
	{
		if (empty($this->attributes['rowspan'])) return 1;
		return ((int) $this->attributes['rowspan'] > 0) ? (int) $this->attributes['rowspan'] : 1;
	}//end rowspan()


	/**
	* Prints the content of this cell complete with TD tag
	*
	* @return void
	* @access public
	*/
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


	/**
	* Prints the backend interface for this table cell
	*
	* @param string	$prefix		prefix for form elements
	* @param int	$tableid	the id of the table this cell is in
	* @param int	$rowid		the id of the row this cell is in
	* @param int	$cellid		the id of this cell
	*
	* @return boolean
	* @access public
	*/
	function paintCell($prefix, $tableid, $rowid, $cellid)
	{
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

		return true;

	}//end paintCell()


	/**
	* Process the backend interface for this table cell
	*
	* @param string	$prefix		prefix for form elements
	* @param int	$tableid	the id of the table this cell is in
	* @param int	$rowid		the id of the row this cell is in
	* @param int	$cellid		the id of this cell
	*
	* @return boolean
	* @access public
	*/
	function processCell($prefix, $tableid, $rowid, $cellid)
	{
		global $bodycopy_action, $bodycopy_name, $bodycopy_data, $bodycopy_saved;
		
		$updated = false;
		if (isset($bodycopy_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'])) {
			foreach($bodycopy_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'] as $name => $value) {
				if ($this->set_attribute($name, $value)) $updated = true;
			}
		}
		return $updated;

	}//end processCell()


	/**
	* Prints the JavaScript needed by the backend functions for this cell
	*
	* @param int	$cellid	the id of this cell
	*
	* @return boolean
	* @access public
	*/
	function print_backend_js_data($cellid)
	{
		?> 
									case <?php echo $cellid?> :
										retVal["attributes"] = '<?php echo var_serialise($this->attributes, true)?>';
									break;
		<?php
	}//end print_backend_js_data()


}//end Class BodyCopy_Table_Cell

?>
