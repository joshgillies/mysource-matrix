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
	* Sets the type of the cells content for editing
	*
	* @param object BodyCopy_Table	&$table	the table object
	* @param string					$type	the type of the cells content
	* @param integer				$rowid	the row the cell is in
	* @param integer				$cellid	the id of the cell
	*
	* @return boolean
	* @access public
	*/
	function setContentType(&$table, $type, $rowid, $cellid)
	{
		$full_type = "content_type_{$type}";
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($full_type);
		$content_type = new $full_type();
		if (!$content_type->create('Row '.($rowid+1).' Cell '.($cellid+1))) return false;
		$this->attributes['linkid'] = $table->createLink($content_type, SQ_LINK_UNITE);
		$this->attributes['content_type'] = $type;
		return true;
	}


	/**
	* Update the link between the cells contentType and the table
	* From time to time it may be nescessary to add or remove rows or columns.
	* We need to make sure when any change happens in the table that the 
	* name remains relevant to where the cell is in the table.
	*
	* @param object BodyCopy_Table	&$table	the table object
	* @param integer				$new_rowid	the row the cell is going in
	* @param integer				$new_cellid	the column the cell is going in
	*
	* @return boolean
	* @access public
	*/
	function updateContentType(&$table, $new_rowid, $new_cellid)
	{
		$link = $table->getLinkById($this->attributes['linkid']);
		if (!$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code'])) return false;
		if (!$content_type->setAttrValue('name', 'Row '.($new_rowid+1).' Cell '.($new_cellid+1))) return false;
		return true;
	}


	/**
	* Delete this bastard
	*
	* @param object BodyCopy_Table	&$table	the table object
	*
	* @return boolean
	* @access public
	*/
	function delete(&$table)
	{
		if (!$table->deleteLink($this->attributes['linkid'])) return false;
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
	* @param object BodyCopy_Table	&$table	the table object
	*
	* @return void
	* @access public
	*/
	function paint(&$table, $prefix=false)
	{
		echo "\t\t<td";
		foreach($this->attributes as $name => $val) {
			if ($val == '') continue;
			if ($name == 'linkid') continue;
			if ($name == 'content_type') continue;
			if (strtolower($name) == 'nowrap') {
				if ($val) echo ' nowrap';
			} else {
				echo ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}
		echo ">\n";
		$link = $table->getLinkById($this->attributes['linkid']);
		$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);
		$content_edit = $content_type->getEditFns();
		if ($prefix) {
			$content_prefix = $prefix.'_content_type_'.$this->attributes['content_type'].'_'.$this->attributes['linkid'];
			$content_edit->paintBackend($content_type, $content_prefix);
		} else {
			ob_start();
			$content_edit->paint($content_type);
			$cell_content = ob_get_contents();
			ob_end_clean();

			// format contents for pretty HTML
			$cell_content = preg_replace("/^/", "\t\t\t", $cell_content);
			$cell_content = preg_replace("/\n/", "\n\t\t\t", $cell_content);
			echo $cell_content;
		}
		echo "\n\t\t</td>\n";
	}


	/**
	* Prints the backend interface for this table cell
	*
	* @param object BodyCopy_Table	&$table	the table object
	* @param string					$prefix		prefix for form elements
	* @param int					$tableid	the id of the table this cell is in
	* @param int					$rowid		the id of the row this cell is in
	* @param int					$cellid		the id of this cell
	*
	* @return boolean
	* @access public
	*/
	function paintCell(&$table, $prefix, $tableid, $rowid, $cellid)
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
		<td <?php echo ($colspaning > 0) ? 'colspan="'.$colspaning.'"' : '';?> id="<?php echo $prefix.'_cell_'.$tableid.'_'.$rowid.'_'.$cellid?>" align="right">
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
	* @param object BodyCopy_Table	&$table		the table object
	* @param string					$prefix		prefix for form elements
	* @param integer				$tableid	the id of the table this cell is in
	* @param integer				$rowid		the id of the row this cell is in
	* @param integer				$cellid		the id of this cell
	*
	* @return boolean
	* @access public
	*/
	function processCell(&$table, $prefix, $tableid, $rowid, $cellid)
	{
		global $bodycopy_action, $bodycopy_name, $bodycopy_data, $bodycopy_saved;
		
		$updated = false;

		// update the contentType
		$link = $table->getLinkById($this->attributes['linkid']);
		if (empty($link)) return false;
		$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);
		$content_edit = $content_type->getEditFns();
		$content_prefix = $prefix.'_content_type_'.$this->attributes['content_type'].'_'.$this->attributes['linkid'];
		if ($content_edit->processBackend($content_type, $content_prefix)) $updated = true;

		if (isset($bodycopy_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'])) {
			foreach($bodycopy_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'] as $name => $value) {
				if ($name == 'type') {
					// changing the content type of this cell
					if ($this->_updateCellType($table, $value)) $updated = true;
					continue;
				}
				if ($this->set_attribute($name, $value)) $updated = true;
			}
		}
		return $updated;

	}//end processCell()


	function _updateCellType(&$table, $new_type='') {
	
		// dont change anything if the new type id blank
		$new_type = trim($new_type);
		if (empty($new_type)) return false;

		// get the contentType
		$link = $table->getLinkById($this->attributes['linkid']);
		if (empty($link)) return false;
		$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);

		// if the new type is the same as the current, change nothing
		$current_type = get_class($content_type);
		if ($current_type == $new_type) return false;

		// morph the content type to the new type
		// first up
		if (!$content_type->morph('content_type')) return false;
		// then down
		if (!$content_type->morph($new_type)) return false;

		$this->set_attribute('content_type', $new_type);

		return true;
	}


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
