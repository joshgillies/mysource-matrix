<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Design_Area
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area extends Asset
{
	/**
	* Holds the names for all variables that are "references" to values in other 
	* @var Array();
	*/
	var $var_references = Array();

	/**
	* Whether the vars in this design area are static - ie they are going to change when viewing difference pages
	* @var boolean
	*/
	var $static_vars = true;

	/*
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Design_Area($assetid=0)
	{
		// No design area is going to be restricted
		$this->_restricted = false;
		$this->Asset($assetid);
	}

	/*
	* Create this asset
	*
	* @param string $id_name   the name of this design area
	*
	* @return boolean
	* @access public
	*/
	function create($id_name)
	{
		$id_name = trim($id_name);

		if ($id_name == '') {
			trigger_error('Unable to create a Design Area with a blank id', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {

			return $this->setAttrValue('id_name',   $id_name);

		}//end if

		return false;

	}//end create()

	/*
	* Returns name of the asset
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName() 
	{
		return $this->attr('id_name');
	}//end _getName();

	/*
	* processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend() 
	{

		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, true);

				$id_name = $this->getAttribute('id_name');
				if ($id_name->processed) {
					return $this->create($id_name->value);
				} else {
					return false;
				}
				break;

			default :
				return parent::processBackend();

		}// end switch

	}// end processBackend()


	/*
	* Returns all the designs that this design area is used in
	*
	* @return Design_Area | null
	* @access public
	*/
	function &getDesigns() 
	{
		$links = $this->getLinks(SQ_LINK_USES, 'design', true, 'minor');
		$designs = Array();
		foreach($links as $link) {
			$designs[] = &$GLOBALS['SQ_RESOLVE']->am->getAsset($link['majorid'], 'design', true);
		}
		return $designs;
	}//end getDesigns()

	/**
	* Set's the values for the var references values (registered in $this->var_references)
	*
	* @param array $values	array of values - Array(name => value)
	*
	* @access public
	*/
	function setVarReferenceValues($values)
	{
		$this->_tmp['var_reference_values'] = $values;
	}//end setVarReferenceValues();

	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name
	*
	* @return mixed
	* @access public
	*/
	function attr($name) 
	{
		if (empty($this->_tmp['var_reference_values'][$name])) {
			return parent::attr($name);
		} else { 
			return $this->_tmp['var_reference_values'][$name];
		}
	}//end attr();

	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @access public
	*/
	function printArea(&$design) 
	{
	}//end printArea()

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	*
	* @access public
	*/
	function printVar($var) 
	{
		echo $this->attr($var);
	}//end printVar()

}//end class
?>
