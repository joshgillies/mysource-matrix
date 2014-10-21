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
* $Id: bodycopy_table_cell.inc,v 1.26 2012/08/30 01:09:05 ewang Exp $
*
*/


/**
* Bodycopy_Table_Cell
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.26 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class BodyCopy_Table_Cell extends MySource_Object
{

	/**
	* an array of attributes for the TD tag
	* @var array
	*/
	public $attributes = Array();


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


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
	public function setAttribute($name, $value)
	{
		$name = strtolower($name);
		if (!isset($this->attributes[$name])) {
			$this->attributes[$name] = '';
		}
		if ($this->attributes[$name] == $value) return FALSE;
		$this->attributes[$name] = $value;
		return TRUE;

	}//end setAttribute()


	/**
	* Sets the type of the cells content for editing
	*
	* @param Bodycopy_Table	$table	the table object
	* @param string			$type	the type of the cells content
	* @param int			$rowid	the row the cell is in
	* @param int			$cellid	the id of the cell
	*
	* @return boolean
	* @access public
	*/
	public function setContentType(Bodycopy_Table $table, $type, $rowid, $cellid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
		$content_link = Array('asset' => &$table, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$content_type = new $type();
		$content_type->setAttrValue('name', 'Row '.($rowid+1).' Cell '.($cellid+1));
		$this->attributes['linkid'] = $content_type->create($content_link);
		if (!$this->attributes['linkid']) return FALSE;
		$this->attributes['content_type'] = $type;
		return TRUE;

	}//end setContentType()


	/**
	* Update the link between the cells contentType and the table
	*
	* From time to time it may be nescessary to add or remove rows or columns.
	* We need to make sure when any change happens in the table that the
	* name remains relevant to where the cell is in the table.
	*
	* @param Bodycopy_Table	$table		the table object
	* @param int			$new_rowid	the row the cell is going in
	* @param int			$new_cellid	the column the cell is going in
	*
	* @return boolean
	* @access public
	*/
	public function updateContentType(Bodycopy_Table $table, $new_rowid, $new_cellid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attributes['linkid'], $table->id);
		if (!$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code'])) {
			return FALSE;
		}

		if (!$content_type->setAttrValue('name', 'Row '.($new_rowid+1).' Cell '.($new_cellid+1))) {
			return FALSE;
		}

		if (!$content_type->saveAttributes()) return FALSE;

		return TRUE;

	}//end updateContentType()


	/**
	* Delete this table cell
	*
	* @param Bopycopy_Table	$table	the table object
	*
	* @return boolean
	* @access public
	*/
	public function delete(Bodycopy_Table $table)
	{
		if (!$table->deleteLink($this->attributes['linkid'])) {
			return FALSE;
		}

		// TODO: this is a replacement for re-assigning $this - needs to be
		// tested
		$this->attributes = Array();
		//unset($this->attributes);
		//$this = new BodyCopy_Table_Cell();
		return TRUE;

	}//end delete()


	/**
	* How many columns does this cell span ??
	*
	* @return int
	* @access public
	*/
	public function colspan()
	{
		if (empty($this->attributes['colspan'])) return 1;
		return ((int) $this->attributes['colspan'] > 0) ? (int) $this->attributes['colspan'] : 1;

	}//end colspan()


	/**
	* How many rows does this cell span ??
	*
	* @return int
	* @access public
	*/
	public function rowspan()
	{
		if (empty($this->attributes['rowspan'])) return 1;
		return ((int) $this->attributes['rowspan'] > 0) ? (int) $this->attributes['rowspan'] : 1;

	}//end rowspan()


	/**
	* Prints the content of this cell complete with TD tag
	*
	* @param Bopycopy_Table	$table		the table object
	* @param string			$prefix		prefix for form elements
	* @param boolean		$editing	are we printing an editing interface
	* @param boolean		$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	public function paint(Bodycopy_Table $table, $prefix='', $editing=FALSE, $generating=FALSE)
	{
		if (!isset($this->attributes['linkid'])) return;

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attributes['linkid'], $table->id);
		if (!empty($link)) {
			$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		} else {
			$content_type = NULL;
		}
		$content_prefix = $prefix.'_content_type_'.$this->attributes['content_type'].'_'.$this->attributes['linkid'];

		echo "\t\t<td";
		foreach ($this->attributes as $name => $val) {
			if ($val == '') continue;
			if ($name == 'linkid') continue;
			if ($name == 'content_type') continue;
			// if we're in edit mode, we don't want to colspan anything at all; the colspan
			// is handled by outer wrapping divs
			if ($name == 'colspan' && $editing && $table->writeAccess('content')) {
				continue;
			}
			if (strtolower($name) == 'nowrap') {
				if ($val) echo ' nowrap';
			} else {
				echo ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}
		if ($editing && $table->writeAccess('content')) {
			echo ' style="margin: 2px; vertical-align:top;" id="'.$content_prefix.'_cell"';
		}
		echo ">\n";

		if (is_null($content_type)) {
			echo sprintf(translate('Content Type Asset Id : #%s not found. Please contact your system administrator'), $link['minorid']);

		} else {
			$content_edit = $content_type->getEditFns();
			if (!empty($prefix)) {
				$content_edit->paintBackend($content_type, $content_prefix, $GLOBALS['SQ_SYSTEM']->backend->out);
			} else {
				ob_start();
					$content_edit->paint($content_type, $editing, $generating);
					$cell_content = ob_get_contents();
				ob_end_clean();

				// format contents for pretty HTML
				$cell_content = preg_replace("/^/", "\t\t\t", $cell_content);
				$cell_content = preg_replace("/\n/", "\n\t\t\t", $cell_content);
				echo $cell_content;
			}
		}
		echo "\n\t\t</td>\n";

	}//end paint()


	/**
	* Prints the backend interface for this table cell
	*
	* @param Bodycopy_Table	$table		the table object
	* @param string			$prefix		prefix for form elements
	* @param int			$tableid	the id of the table this cell is in
	* @param int			$rowid		the id of the row this cell is in
	* @param int			$cellid		the id of this cell
	*
	* @return boolean
	* @access public
	*/
	public function paintCell(Bodycopy_Table $table, $prefix, $tableid, $rowid, $cellid)
	{
		$attribute_list = '';
		$colspaning = 0;
		$dir_attribute = '';
		foreach ($this->attributes as $name => $val) {
			if ($val == '') continue;
			if (strtolower($name) == 'nowrap') {
				if ($val) $attribute_list .= ' nowrap';
			} else {
				if (strtolower($name) == 'colspan') {
					$colspaning = $val;
				}
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
			if ($name == 'dir') $dir_attribute = $val;
		}

		// print out the edit buttons

		Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_edit_table_cell_properties('$prefix', $tableid, $rowid, $cellid);", 'Edit Cell '.($cellid + 1).' - '.($rowid+1).'\'s Properties', (($attribute_list) ? 'Current Properties :<br>'.$attribute_list : ''), 'cell_properties', '[Edit Cell Properties]');

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attributes['linkid'], $table->id);
		if (!empty($link)) {
			$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		} else {
			$content_type = NULL;
		}

		// set the dir attribute to one inherited from above if there is no specified for cell
		if (empty($dir_attribute) && isset($this->_tmp['dir_attr'])) {
			$dir_attribute = $this->_tmp['dir_attr'];
		}

		if (!is_null($content_type)) {
			$content_edit = $content_type->getEditFns();
			$content_prefix = $prefix.'_content_type_'.$this->attributes['content_type'].'_'.$this->attributes['linkid'];
			if (method_exists($content_edit,'setDirAttr') && !empty($dir_attribute)) {
				$content_edit->setDirAttr($dir_attribute);
			}
			$content_edit->paintBackendIcons($content_type, $content_prefix, $GLOBALS['SQ_SYSTEM']->backend->out);
		}

		return TRUE;

	}//end paintCell()


	/**
	* Prints the backend status icons, e.g. htmltidy status
	*
	* @param Bodycopy_Table	$table		the table object
	* @param string			$prefix		prefix for form elements
	* @param int			$tableid	the id of the table this cell is in
	* @param int			$rowid		the id of the row this cell is in
	* @param int			$cellid		the id of this cell
	*
	* @return boolean
	* @access public
	*/
	public function paintCellBackendStatusIcons(Bodycopy_Table $table, $prefix, $tableid, $rowid, $cellid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attributes['linkid'], $table->id);
		if (!empty($link)) {
			$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		} else {
			$content_type = NULL;
		}

		if (!is_null($content_type)) {
			$content_edit = $content_type->getEditFns();
			$content_prefix = $prefix.'_content_type_'.$this->attributes['content_type'].'_'.$this->attributes['linkid'];
			$content_edit->paintBackendStatusIcons($content_type, $content_prefix, $GLOBALS['SQ_SYSTEM']->backend->out);
		}

		return TRUE;

	}//end paintCellBackendStatusIcons()


	/**
	* Process the backend interface for this table cell
	*
	* @param Bodycopy_Table	$table		the table object
	* @param string			$prefix		prefix for form elements
	* @param int			$tableid	the id of the table this cell is in
	* @param int			$rowid		the id of the row this cell is in
	* @param int			$cellid		the id of this cell
	* @param array			$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	public function processCell(Bodycopy_Table $table, $prefix, $tableid, $rowid, $cellid, Array $bc_saved)
	{
		$updated = FALSE;

		// update the contentType
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attributes['linkid'], $table->id);
		if (empty($link)) return FALSE;
		$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		$content_edit = $content_type->getEditFns();
		$content_prefix = $prefix.'_content_type_'.$this->attributes['content_type'].'_'.$this->attributes['linkid'];
		if ($content_edit->processBackend(Array(), $content_type, $content_prefix)) {
			$updated = TRUE;
		}
		if (!$content_type->saveAttributes()) return FALSE;

		if (isset($bc_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'])) {
			foreach ($bc_saved[$prefix][$tableid]['rows'][$rowid]['cells'][$cellid]['attributes'] as $name => $value) {
				if ($name == 'type') {
					// changing the content type of this cell
					if ($this->_updateCellType($table, $value)) {
						$updated = TRUE;
					}
					continue;
				}
				if ($this->setAttribute($name, $value)) {
					$updated = TRUE;
				}
			}
		}
		return $updated;

	}//end processCell()


	/**
	* Change the type of this cell to the passed content type
	*
	* @param object	&$table		the table object
	* @param string	$new_type	the name of the new cell type
	*
	* @return boolean
	* @access private
	*/
	protected function _updateCellType(Bodycopy_Table $table, $new_type='')
	{

		// dont change anything if the new type id blank
		$new_type = trim($new_type);
		if (empty($new_type)) return FALSE;

		// get the contentType
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attributes['linkid'], $table->id);
		if (empty($link)) return FALSE;
		$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

		// if the new type is the same as the current, change nothing
		$current_type = get_class_lower($content_type);
		if ($current_type == $new_type) return FALSE;

		// morph the content type to the new type
		// first up
		$morphed = $content_type->morph('content_type');
		if (!$morphed) return FALSE;

		// then down
		$content_type = $morphed->morph($new_type);
		if (!$content_type) return FALSE;

		$this->setAttribute('content_type', $new_type);

		return TRUE;

	}//end _updateCellType()


	/**
	* Prints the JavaScript needed by the backend functions for this cell
	*
	* @param int	$cellid	the id of this cell
	*
	* @return boolean
	* @access public
	*/
	public function printBackendJsData($cellid)
	{
		?>

									case <?php echo $cellid?> :
										retVal["attributes"] = '<?php echo var_serialise($this->attributes, TRUE)?>';
									break;
		<?php

	}//end printBackendJsData()


	/**
	* Sets the tmp var for dir attribute
	*
	* @param string					$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function setDirAttr($dir_attribute)
	{
		$this->_tmp['dir_attr'] = $dir_attribute;

	}//end setDirAttr()


}//end class

?>
