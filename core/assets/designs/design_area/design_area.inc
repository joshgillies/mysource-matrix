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
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Design_Area($assetid=0)
	{
		$this->_ser_atrs   = true; // we have serialise type vars
		// No design area is going to be restricted
		$this->_restricted = false;
		$this->Asset($assetid);
	}

	/**
	* Create this asset
	*
	* @param array(string)	&$link	information used to create the initial link
	* @param string $id_name   the name of this design area
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $id_name)
	{
		$id_name = trim($id_name);

		if ($id_name == '') {
			trigger_error('Unable to create a Design Area with a blank id', E_USER_WARNING);
			return false;
		}

		if (parent::create($link)) {

			return $this->setAttrValue('id_name', $id_name);

		}//end if

		return false;

	}//end create()

	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('id_name');
	}//end _getName();

	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		$this->_tmp['regenerate_design'] = false;
		$processed = parent::processBackend($o, $link);
		if ($processed && $this->_tmp['regenerate_design']) {
			$designs = &$this->getDesigns();
			for($i = 0, $max = count($designs); $i < $max; $i++) {
				$designs[$i]->generateDesignFile(false);
			}// end for
		}// end if

		return $processed;

	}// end processBackend()

	/**
	* Returns all the designs that this design area is used in
	*
	* @return Design_Area | null
	* @access public
	*/
	function &getDesigns()
	{
		$links = $this->getLinks(SQ_LINK_USES, 'design', false, 'minor');
		$designs = Array();
		foreach($links as $link) {
			$designs[] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['type_code'], true);
		}
		return $designs;
	}//end getDesigns()


	/**
	* Returns an array of Array(id_name => Array(var)) for all var references that are needed by this design area
	*
	* @access public
	*/
	function getVarReferences()
	{
		$var_refs = Array();
		foreach($this->vars['var_references']['value'] as $var_name => $info) {
			if (!isset($var_refs[$info['id_name']])) $var_refs[$info['id_name']] = Array();
			$var_refs[$info['id_name']][] = $info['var'];
		}// end foreach

		return $var_refs;

	}//end getVarReferences();


	/**
	* Set's the values for the var references values (registered in $this->var_references)
	*
	* @param array $var_ref_values	array of values - Array(id_name => Array(var => [value]))
	*
	* @access public
	*/
	function setVarReferenceValues($var_ref_values)
	{
		if (!isset($this->_tmp['var_reference_values'])) $this->_tmp['var_reference_values'] = Array();
		foreach($this->vars['var_references']['value'] as $var_name => $info) {
			if (isset($var_ref_values[$info['id_name']][$info['var']])) {
				$this->_tmp['var_reference_values'][$var_name] = $var_ref_values[$info['id_name']][$info['var']];
			}
		}// end foreach

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
		if (isset($this->_tmp['var_reference_values'][$name])) {
			return $this->_tmp['var_reference_values'][$name];
		} else {
			return parent::attr($name);
		}
	}//end attr();

	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		$edit_fns = $this->getEditFns();
		return $edit_fns->_createDesignFileOutput($this->attr('contents'), $this, $design);
	}//end printArea()

	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param Array	$args	array of any extra arguments that existed in the print command
	*
	* @access public
	*/
	function printVar($var, $args)
	{
		echo $this->attr($var);
	}//end printVar()

}//end class
?>
