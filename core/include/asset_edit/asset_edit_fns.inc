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
* @package Resolve
*/
class Asset_Edit_Fns extends Resolve_Object
{

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $tabs = Array(	'details'     => Array('name' => 'Details',
											   'icon' => 'create',
											   'force_unlock' => true
											  ),
						'permissions' => Array('name' => 'Permissions',
											   'icon' => 'users',
											   'force_unlock' => true
											  ),
						'workflow'    => Array('name' => 'Workflow',
											   'icon' => 'web',
											   'force_unlock' => false
											  ),
						'locking'    => Array('name' => 'Locking',
											  'icon' => 'preview',
											  'force_unlock' => true
											  )
					  );


	/**
	* Constructor
	*
	*/
	function Asset_Edit_Fns() 
	{
		$this->Resolve_Object();
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
	function paintPaths(&$asset, &$o, $prefix) 
	{

		$paths = $asset->getWebPaths();

		foreach($paths as $path) {
			text_box($prefix.'_paths[]', $path, 25);
			echo '<br/>';
		}

		text_box($prefix.'_paths[]', '', 25);
		echo '<br/>';
		
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
		$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']));
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
