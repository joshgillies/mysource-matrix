<?php

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Asset_Edit_Fns extends Resolve_Object
{

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
			$o->text_box($prefix.'_path[]', $path, 25);
			echo '<br>';
		}

		$o->text_box($prefix.'_path[]', '', 25);
		echo '<br>';
		
	}// end paintPaths()

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

		$urls = $asset->getWebPaths(true);
		foreach($urls as $url) {
			echo '<a target="_blank" href="'.$url.'">'.$url.'</a><br>';
		}
		
	}// end paintURLs()


}//end class
?>
