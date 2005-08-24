<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: design_area.inc,v 1.40.2.5 2005/08/24 06:58:53 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Design_Area
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.40.2.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Design_Area($assetid=0)
	{
		$this->_ser_attrs = true; // we have serialise type vars
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Design areas ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return false;

		$id_name = trim($this->attr('id_name'));
		if ($id_name == '') {
			trigger_localised_error('CORE0187', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return false;
		}

		return true;

	}//end _preCreateCheck()


	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=true)
	{
		// don't update the parent asset when this asset is updated as it starts a recusion
		// that takes way too long
		return parent::_updated(false);
	}//end _updated()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['parsing'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		$name = $this->attr('id_name');
		// if the attribute is empty, let the parent take care of setting the name
		if ($name == '') $name = parent::_getName($short_name);

		return $name;

	}//end _getName()


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
		if (parent::processBackend($o, $link)) {
			$designs = &$this->getDesigns();
			for ($i = 0, $max = count($designs); $i < $max; $i++) {
				$designs[$i]->generateDesignFile(false);
			}//end for

			return true;

		} else {

			return false;

		}//end if

	}//end processBackend()


	/**
	* Returns all the designs that this design area is used in
	*
	* @return Array() array of Design objects
	* @access public
	*/
	function &getDesigns()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design', false, 'minor');
		$designs = Array();
		foreach ($links as $link) {
			$designs[] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code'], true);
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
		foreach ($this->vars['var_references']['value'] as $var_name => $info) {
			if (!isset($var_refs[$info['id_name']])) {
				$var_refs[$info['id_name']] = Array();
			}
			$var_refs[$info['id_name']][] = $info['var'];
		}//end foreach

		return $var_refs;

	}//end getVarReferences()


	/**
	* Set's the values for the var references values (registered in $this->var_references)
	*
	* @param array $var_ref_values	array of values - Array(id_name => Array(var => [value]))
	*
	* @access public
	*/
	function setVarReferenceValues($var_ref_values)
	{
		if (!isset($this->_tmp['var_reference_values'])) {
			$this->_tmp['var_reference_values'] = Array();
		}
		foreach ($this->vars['var_references']['value'] as $var_name => $info) {
			if (isset($var_ref_values[$info['id_name']][$info['var']])) {
				$this->_tmp['var_reference_values'][$var_name] = $var_ref_values[$info['id_name']][$info['var']];
			}
		}//end foreach

	}//end setVarReferenceValues()


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

	}//end attr()


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	* And cache the output using the Cache Manager.
	*
	* @param object Design	$design the design for which we are currently printing
	* @param boolean 			$cache true this design area should be cached
	*
	* @return boolean
	* @access public
	*/
	function printCachedArea(&$design, $cache=false)
	{
		$instance_id = '_'.$this->id.'_'.$design->id;

		if ($cache && $this->canBeCached()) {
			echo '
			<?php
				$cm = &$GLOBALS[\'SQ_SYSTEM\']->am->getSystemAsset(\'cache_manager\');
				$asset_contents'.$instance_id.' = $cm->loadFromCache('.$this->id.', \''.$this->type().'\', \''.$design->id.'\');

				if (empty($asset_contents'.$instance_id.')) {
					ob_start();
			?>
			';
		}

		$ret = $this->printArea($design);

		if ($cache && $this->canBeCached()) {
			echo '
			<?php
						$asset_contents'.$instance_id.' = ob_get_contents();
					ob_end_clean();
					$cm->saveToCache('.$this->id.', \''.$this->type().'\', \''.$design->id.'\', $asset_contents'.$instance_id.');
				}

				echo $asset_contents'.$instance_id.';
			?>
			';
		}

		return $ret;

	}//end printCachedArea()


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
		echo $this->_escapeVar($this->attr($var), $args);

	}//end printVar()


	/**
	* Escapes a simple var (if requested) that is going to be printed in the PHP
	*
	* @param string	$value	the value
	* @param Array	$args	array of any extra arguments that existed in the print command
	*
	* @access public
	*/
	function _escapeVar($value, $args)
	{
		if (empty($args['escape']) || strtolower($args['escape']) == 'no') {
			return $value;
		} else {
			return 'addslashes('.$value.')';
		}

	}//end _escapeVar()


	/**
	 * Returns true if this content generated by this design area can be cached
	 *
	 * @return boolean
	 * @access public
	 */
	function canBeCached()
	{
		return 1;

	}//end canBeCached()


}//end class

?>
