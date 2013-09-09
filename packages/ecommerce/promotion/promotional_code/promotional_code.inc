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
* $Id: promotional_code.inc,v 1.3 2013/09/09 08:35:57 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Promotional_Code
*
* This asset stores the promotional code, the assets to apply the code and rules to calculate the discount
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Code extends Asset
{
	
	
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor
	
	
	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Promotional codes ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}
		
		return TRUE;
		
	}//end _preCreateCheck()
	
	
	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()
	
	
	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');
		
	}//end _getName()
	
	
	/**
	* Save attributes set with setAttrValue to the database
	* 
	* This method is overrided here because we want to update the link values of the links between this promotional code asset
	* and the applied assets when the promotional code is being changed. We use the link value to match the promotional code
	* in ecommerce form and cart quickly.
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see Asset::saveAttributes()
	*/
	public function saveAttributes($dont_run_updated=FALSE)
	{
		// If the promotional code is being changed, update the link values of the links between this promotional code asset and the applied assets
		if (isset($this->_tmp['vars_set']['code']['old_value'])) {
			$old_link_value = $this->_tmp['vars_set']['code']['old_value'];
			$new_link_value = $this->attr('code');
			$am = $GLOBALS['SQ_SYSTEM']->am;
			$links = $am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', $old_link_value);
			
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			
			try {
				foreach ($links as $link) {
					if (!$am->updateLink($link['linkid'], $link['link_type'], $new_link_value)) {
						throw new Exception("The value of the notice link #{$link['linkid']} could not be updated to $new_link_value");
					}
				}
				
				if (!parent::saveAttributes($dont_run_updated)) {
					throw new Exception('The attributes could not be saved');
				}
				
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				
				return TRUE;
				
			} catch (Exception $e) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				throw $e;
			}
			
		} else {
			return parent::saveAttributes($dont_run_updated);
		}
		
	}//end saveAttributes()
	
	
	/**
	 * Return the old value of the $name attribute before the setAttrValue() method is invoked.
	 * If no new value is set, the current value is returned.
	 * 
	 * @param string $name	The name of the attribute to get its old value
	 * 
	 * @return mixed
	 * @access public
	 * @see Asset::setAttrValue()
	 */
	public function getOldAttrValue($name)
	{
		if (isset($this->_tmp['vars_set'][$name]['old_value'])) {
			$value = $this->_tmp['vars_set'][$name]['old_value'];
		} else {
			$value = $this->attr('code');
		}
		
		return $value;
		
	}//end getOldAttrValue()
	
	
	/**
	 * Return the rules that are applied to this promotional code
	 * 
	 * @return array
	 * @access public
	 */
	public function getRules()
	{
		return $this->attr('rules');
		
	}//end getRules()
	
	
	/**
	 * Process the promotional code rules with the input asset data
	 * 
	 * @param array &$asset_data	The input asset data
	 * 
	 * @return void
	 * @access public
	 */
	public function process(&$asset_data)
	{
		$rules = $this->attr('rules');
		$am = $GLOBALS['SQ_SYSTEM']->am;
		
		foreach ($rules as $rule) {
			$rule_type_code = $rule['rule_type_code'];
			$am->includeAsset($rule_type_code);
			$rule_asset = new $rule_type_code();
			$rule_asset->evaluate($asset_data, $rule['rule_data']);
		}
		
	}//end process()
	
	
}//end class

?>
