<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Site_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Site_Edit_Fns extends Folder_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Site_Edit_Fns() 
	{
		$this->Folder_Edit_Fns();
	}

	/**
	* Paint the interface for the asset paths
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintURLs(&$asset, &$o, $prefix) 
	{

		$paths = $asset->getWebPaths();

		$ports = Array('

		foreach($paths as $path) {
			$o->text_box($prefix.'_path_urls[]', $path, 25);
			$o->text_box($prefix.'_path_urls[]', $path, 25);
			$o->text_box($prefix.'_path_ports[]', $path, 25);
			echo '<br>';
		}

		$o->text_box($prefix.'_path[]', '', 25);
		echo '<br>';
		
	}// end paintURLs()


}//end class
?>
