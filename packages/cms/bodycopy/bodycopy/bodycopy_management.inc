<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Bodycopy_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array('added'     => '0.0.1',
											'type'      => 'text',  'default'   => 'BodyCopy',
											'update_fn' => ''
											)
							);
	}//end constructor


	/**
	* Install
	*
	* @access public
	*/
	function _install()
	{
		$ok = parent::_install();
		pre_echo($this->getAssetType().' Specific install stuff here');
		return $ok;

	}//end install


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		$ok = parent::_upgrade($current_version);
		pre_echo($this->getAssetType().' Specific upgrade stuff here');

		switch($current_version) {
			case '0.1': 
				// 0.1 to 0.2 upgrade stuff here
			case '0.2': 
				// 0.2 to 0.3 upgrade stuff here
			case '0.3': 
				// 0.3 to 0.4 upgrade stuff here
			case '0.4': 
				// 0.4 to 0.5 upgrade stuff here
		}

		return $ok;

	}//end upgrade()


	/**
	* Gets a list of all files that the bodycopy wants web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList() {
		return Array('images/down_arrow.gif',
					 'images/embedded_object.gif',
					 'images/no_content.gif',
					 'images/up_arrow.gif',
					 'images/icons/copy.gif',
					 'images/icons/delete.gif',
					 'images/icons/edit_cell_properties.gif',
					 'images/icons/edit_content.gif',
					 'images/icons/edit.gif',
					 'images/icons/edit_row_properties.gif',
					 'images/icons/edit_table_properties.gif',
					 'images/icons/insert_column_left.gif',
					 'images/icons/insert_column_right.gif',
					 'images/icons/insert.gif',
					 'images/icons/insert_row_above.gif',
					 'images/icons/insert_row_below.gif',
					 'images/icons/insert_table.gif',
					 'images/icons/new_page.gif',
					 'images/icons/paste.gif',
					 'images/icons/reorder_cols.gif',
					 'images/icons/reorder_rows.gif',
					 'images/icons/reorder_tables.gif',
					 'images/icons/separator_horizontal.gif',
					 'images/icons/separator_vertical.gif',
					 'js/bodycopy_edit_tables.js'
					 );
	}//end _getFileList()

}//end class
?>
