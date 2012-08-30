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
* $Id: condition_user_ip_management.inc,v 1.6 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once dirname(__FILE__).'/condition_user_ip_edit_fns.inc';

/**
* Condition_User_IP_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_IP_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor


	/**
	* Upgrade this asset to the current version
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.2', '<')) {
			// Need to go through all users who have user group restrictions set
			pre_echo('STARTING UPGRADE OF '.$asset_name.' FROM VERSION '.$current_version.' TO VERSION 0.2');

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// Get a list of users...
			$users = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('user', FALSE, TRUE);
			pre_echo('Updating User restrictions - '.count($users).' asset(s)...');
			foreach ($users as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				$restrictions = $asset->attr('restrictions');
				foreach ($restrictions as $group_assetid => $group_restrictions) {
					foreach ($group_restrictions as $key => $restriction) {
						if ($restriction['name'] == $this->getAssetType()) {
							$restriction = $this->_convertConditionData($restriction);

							if (!is_null($restriction)) {
								$restrictions[$group_assetid][$key] = $restriction;
							} else {
								unset($restrictions[$group_assetid][$key]);
							}
						}
					}
				}

				$asset->setAttrValue('restrictions', $restrictions);

				if (!$asset->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					pre_echo($asset_name.' UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach

			// Need to go through all paint layout bodycopies who have conditional keywords set up
			$pl_bodycopies = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('paint_layout_bodycopy', FALSE, TRUE);
			pre_echo('Updating Paint Layout Bodycopy conditions - '.count($pl_bodycopies).' asset(s)...');
			foreach ($pl_bodycopies as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				$conditional_keywords = $asset->attr('conditional_keywords');
				foreach ($conditional_keywords as $condition_name => $parameters) {
					if ($parameters['type_code'] == $this->getAssetType()) {
						$parameters['restriction'] = $this->_convertConditionData($parameters['restriction']);
					}

					$conditional_keywords[$condition_name] = $parameters;
				}//end foreach

				$asset->setAttrValue('conditional_keywords', $conditional_keywords);

				if (!$asset->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					pre_echo($asset_name.' UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo($asset_name.' SUCCESSFULLY UPGRADED TO VERSION 0.2');

		}//end if

		return TRUE;

	}//end _upgrade()


	/**
	* Convert condition data
	*
	* @param array	$restriction	the restriction data to convert
	*
	* @return array
	* @access private
	*/
	function _convertConditionData($restriction)
	{
		$condition_data = $restriction['condition_data'];

		if (isset($condition_data['user_ip'])) {
			$ip_octets = explode('.', $condition_data['user_ip']);
			$subnet_octets = Array(255, 255, 255, 255);
			$valid = TRUE;
			$non_star_found = FALSE;

			// loop through each octet, backwards
			for ($i = 3; $i >= 0; $i--) {
				// if a '*' is found, make sure we haven't had an actual number yet,
				// then switch the subnet and IP octets to zero
				if ($ip_octets[$i] === '*') {
					if ($non_star_found) {
						trigger_error('Cannot upgrade IP condition "'.$condition_data['user_ip'].' as it cannot be represented in a IP/subnet format - ignoring this condition', E_USER_NOTICE);
						$valid = FALSE;
						break;
					} else {
						$subnet_octets[$i] = '0';
						$ip_octets[$i] = '0';
					}
				} else {
					$non_star_found = TRUE;
				}
			}

			// is it valid?
			if ($valid) {
				$match = $restriction['match'];
				$condition_data['user_ip_ranges'] = Array(
														Array(
															'ip_address'	=> implode('.', $ip_octets),
															'subnet'		=> implode('.', $subnet_octets),
															'grant'			=> $match,
														),
													);
				$condition_data['default_grant'] = 1 - $match;
				$restriction['match'] = TRUE;
				unset($condition_data['user_ip']);
				$restriction['condition_data'] = $condition_data;
			} else {
				$restriction = NULL;
			}

		}//end if

		return $restriction;

	}//end _convertConditionData()


}//end class

?>
