<?php
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Edit_Fns extends MySource_Object
{

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $static_screens = Array('details'         => Array('name' => 'Details',
													   'force_unlock' => true
													  ),
								'permissions'     => Array('name' => 'Permissions',
													   'force_unlock' => true
													  ),
								'workflow'        => Array('name' => 'Workflow',
													   'force_unlock' => false
													  ),
								'metadataSchemas' => Array('name' => 'Metadata Schemas',
													   'force_unlock' => true
													  ),
								'metadata'        => Array('name' => 'Metadata',
													   'force_unlock' => true
													  ),
								'dependants'      => Array('name' => 'Dependants',
													   'force_unlock' => true
													  ),
								'linking'         => Array('name' => 'Linking',
													   'force_unlock' => true
													  ),
								'settings'        => Array('name' => 'Settings',
													   'force_unlock' => false
													  ),
								'history'         => Array('name' => 'History',
													   'force_unlock' => true
													  ),
								);


	/**
	* Constructor
	*
	*/
	function Asset_Edit_Fns()
	{
		$this->MySource_Object();

	}//end constructor


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
	function paintPaths(&$asset, &$o, $prefix)
	{

		$wa = $asset->writeAccess();

		$paths = $asset->getWebPaths();

		foreach($paths as $path) {
			if ($wa) text_box($prefix.'_paths[]', $path, 25);
			else echo $path;
			echo '<br/>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 25);

	}// end paintPaths()


	/**
	* Processes the Paths posted from the paintPaths fn above
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPaths(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_paths'])) return false;
		return $asset->saveWebPaths(array_values($_POST[$prefix.'_paths']));

	}// end processPaths()


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
		$urls = $asset->getLookups('url');
		foreach($urls as $url) {
			echo $url, '<br/>';
		}

	}// end paintURLs()


}//end class

?>