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
* $Id: design_area.inc,v 1.68 2012/08/30 01:09:05 ewang Exp $
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
* @version $Revision: 1.68 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE; // we have serialise type vars
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Design areas ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$id_name = trim($this->attr('id_name'));
		if ($id_name == '') {
			trigger_localised_error('CORE0187', sprintf(translate('Unable to create a %s with a blank id name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

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
	public function _updated($update_parents=TRUE)
	{
		// don't update the parent asset when this asset is updated as it starts a recusion
		// that takes way too long
		return parent::_updated(FALSE);

	}//end _updated()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['parsing'] = $lock_types['all'];
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		$name = $this->attr('id_name');
		// if the attribute is empty, let the parent take care of setting the name
		if ($name == '') $name = parent::_getName($short_name);

		return $name;

	}//end _getName()


	/**
	* Processes a backend submission from this asset, returns TRUE if all OK
	*
	* @param object	&$o		Backend outputter
	* @param array	&$link	Information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	public function processBackend(Backend_Outputter $o, Array &$link)
	{
		if (parent::processBackend($o, $link)) {
			$designs = $this->getDesigns();
			for ($i = 0, $max = count($designs); $i < $max; $i++) {
				$designs[$i]->generateDesignFile(FALSE);
			}//end for

			return TRUE;

		} else {

			return FALSE;

		}//end if

	}//end processBackend()


	/**
	* Returns all the designs that this design area is used in
	*
	* @return array
	* @access public
	*/
	public function &getDesigns()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design', FALSE, 'minor');
		$designs = Array();
		foreach ($links as $link) {
			$designs[] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code'], TRUE);
		}
		return $designs;

	}//end getDesigns()


	/**
	* Returns an array of Array(id_name => Array(var)) for all var references that are needed by this design area
	*
	* @return array
	* @access public
	*/
	public function getVarReferences()
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
	* Sets the values for the var references values (registered in $this->var_references)
	*
	* @param array	$var_ref_values	array of values - Array(id_name => Array(var => [value]))
	*
	* @return void
	* @access public
	*/
	public function setVarReferenceValues(Array $var_ref_values)
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
	* @param string	$name	Name of attribute to retrieve
	*
	* @return string
	* @access public
	*/
	public function attr($name)
	{
		if (isset($this->_tmp['var_reference_values'][$name])) {
			return $this->_tmp['var_reference_values'][$name];
		} else {
			return parent::attr($name);
		}

	}//end attr()


	/**
	* Get the names of attributes for this DA that cannot be set in the parse file
	*
	* Design Area attributes that are not mentioned in the parse file are normally set to defaults
	* when parsing the design.  Attributes returned by this function are exempted from that process.
	*
	* @return array
	* @access public
	*/
	public function getProtectedAttrs()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes('design_area', Array('name'));

	}//end getProtectedAttrs()


//--        PARSE FILE GENERATION        --//


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args)
	{
		echo $this->_escapeVar($this->attr($var), $args);

	}//end printVar()


	/**
	* Escapes a simple var (if requested) that is going to be printed in the PHP
	*
	* @param string	$value	the value
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return string
	* @access public
	*/
	protected function _escapeVar($value, Array $args)
	{
		if (empty($args['escape']) || strtolower($args['escape']) == 'no') {
			return $value;
		} else {
			return 'addslashes('.$value.')';
		}

	}//end _escapeVar()


	/**
	* Safely get a PHP literal string (including the quotes) for printing to the design file
	*
	* Prevents code injection
	*
	* @param string	$string	The contents for the literal string
	*
	* @return string
	* @access protected
	*/
	protected function _safeStringLiteral($string)
	{
		return var_export((string)$string, TRUE);

	}//end _safeStringLiteral()


	/**
	* Returns TRUE if this content generated by this design area's code in the design file can be cached
	*
	* This function should be overriden by children if they don't want the cache=1 attribute to be accepted
	*
	* @return boolean
	* @access protected
	*/
	protected function _canBeCached()
	{
		return TRUE;

	}//end _canBeCached()


	/**
	* Return whether this DA has been given sufficient details to be able to print stuff
	*
	* This function can be overriden by children to enforce required attributes etc
	*
	* @return boolean
	* @access protected
	*/
	protected function _canInit()
	{
		return TRUE;

	}//end _canInit()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* The protected version of this function, rather than this one, should be overridden by children
	*
	* @return void
	* @access public
	*/
	public function printInitCode()
	{
		$design =& $GLOBALS['DESIGN_BEING_GENERATED'];
		$instance_id = '_'.$this->id.'_'.$design->id;
		if ($this->attr('cache') && $this->_canBeCached()) {
			echo '
			<?php
				$cm = $GLOBALS[\'SQ_SYSTEM\']->am->getSystemAsset(\'cache_manager\');
				$asset_contents'.$instance_id.' = $cm->loadFromCache('.$this->id.', \''.$this->type().'\', \''.$design->id.'\', !'.$this->attr('cache_globally').');

				if ($asset_contents'.$instance_id.' === FALSE) {
			?>
			';
		}
		$this->_printInitCode();

		if ($this->attr('cache') && $this->_canBeCached()) {
			echo '
			<?php
				}
			?>
			';
		}

	}//end printInitCode()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* This function can be overridden by children
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{

	}//end _printInitCode()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area. The protected version of this function, rather that this one,
	* should overridden by children
	*
	* @return void
	* @access public
	*/
	public function printPaintingCode()
	{
		if (!array_get_index($this->_tmp, 'ready_to_paint')) {
			return;
		}
		$design =& $GLOBALS['DESIGN_BEING_GENERATED'];
		$instance_id = '_'.$this->id.'_'.$design->id;
		if ($this->attr('cache') && $this->_canBeCached()) {
			echo '
			<?php
				if ($asset_contents'.$instance_id.' === FALSE) {
					ob_start();
			?>
			';
		}

		$this->_printPaintingCode();

		if ($this->attr('cache') && $this->_canBeCached()) {
			echo '
			<?php
					$asset_contents'.$instance_id.' = ob_get_contents();
					ob_end_clean();
					$cm = $GLOBALS[\'SQ_SYSTEM\']->am->getSystemAsset(\'cache_manager\');
					$cm->saveToCache('.$this->id.', \''.$this->type().'\', \''.$design->id.'\', $asset_contents'.$instance_id.', !'.$this->attr('cache_globally').');
				}
				echo $asset_contents'.$instance_id.';
			?>
			';
		}

	}//end printPaintingCode()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area. This function can be overridden (partially) by children
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		$contents = $this->attr('contents');
		$current_element = NULL;

		// remove any leading or trailing whitespace elements from the contents
		// first, the leaders
		if (count($contents) > 0) {
			$current_element = array_shift($contents);
			while(($current_element['_type'] == 'HTML') && (trim($current_element['contents']) == '')) {
				if (count($contents) == 0) {
					$current_element = NULL;
					break;
				}
				$current_element = array_shift($contents);
			}
		}

		if (!is_null($current_element)) {
			array_unshift($contents, $current_element);
		}

		// now the trailers...
		if (count($contents) > 0) {
			$current_element = array_pop($contents);
			while(($current_element['_type'] == 'HTML') && (trim($current_element['contents']) == '')) {
				if (count($contents) == 0) {
					$current_element = NULL;
					break;
				}
				$current_element = array_pop($contents);
			}
		}

		if (!is_null($current_element)) {
			array_push($contents, $current_element);
		}

		// Now print out what we have left
		foreach ($contents as $item) {
			$this->_printContentItemCode($item);
		}

	}//end _printPaintingCode()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$item	An entry from our contents array
	*
	* @return void
	* @access private
	*/
	protected function _printContentItemCode(Array $item)
	{
		if ($item['_type'] == 'HTML') {
			// just some HTML content to echo out
			echo $item['contents'];
		} else if ($item['_type'] == 'TAG') {
			if ($item['operation'] == 'print') {
				$design =& $GLOBALS['DESIGN_BEING_GENERATED'];
				if (empty($item['attributes']['id_name'])) {
					$args = $item['attributes'];
					unset($args['id_name']);
					unset($args['var']);
					$this->printVar($item['attributes']['var'], $args);
				} else {
					if ($item['attributes']['id_name'] == '__global__') {
						// printing a global var
						$args = $item['attributes'];
						unset($args['id_name']);
						unset($args['var']);
						$design->printVar($item['attributes']['var'], $args);
					} else {
						// referencing another design area (which should already have been defined)
						$da = $design->getDesignArea($item['attributes']['id_name']);
						if (is_null($da)) {
							trigger_localised_error('CORE0153', sprintf(translate('Design Area "%s" not found, ignoring print operation'), $item['attributes']['id_name']), E_USER_NOTICE);
						} else {
							if (isset($item['attributes']['var'])) {
								// printing a particular var from the design area
								$args = $item['attributes'];
								unset($args['id_name']);
								unset($args['var']);
								$da->printVar($item['attributes']['var'], $args);
							} else {
								// printing the whole other design area
								$da->printPaintingCode();
							}
						}
					}
				}
			}//end if operation
		}//end if type = TAG

	}//end _printContentItemCode()


	/**
	* Print the replacement for the AREA tag in the parse file
	*
	* This will always print the init code for the DA;  unless the Area tag has a
	* negative print attribute the painting code is printed too.
	*
	* @return void
	* @access public
	*/
	public function printAreaCode()
	{
		if ($this->_canInit()) {
			$this->printInitCode();
			$this->_tmp['ready_to_paint'] = TRUE;
			if ($this->attr('print')) $this->printPaintingCode();
		}

	}//end printAreaCode()


}//end class
?>
