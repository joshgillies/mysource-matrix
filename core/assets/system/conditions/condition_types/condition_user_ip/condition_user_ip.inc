<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: condition_user_ip.inc,v 1.7 2006/12/06 04:23:59 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';
require_once dirname(__FILE__).'/condition_user_ip_edit_fns.inc';


/**
* Condition_User_IP
*
* Purpose
*     This condition compares the IP address of the current visitor against a
*     specified comparison address
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_IP extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_User_IP($assetid=0)
	{
		$this->Condition($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* @param object	&$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	function evaluate(&$asset, $condition_data)
	{
		if (!isset($condition_data['user_ip_ranges'])) {
			return FALSE;
		}

		$remote_ip = $_SERVER['REMOTE_ADDR'];
		$grant = array_get_index($condition_data, 'default_grant', FALSE);

		// sort the IP ranges by subnet mask order
		$ip_ranges = Condition_User_IP_Edit_Fns::_sortByCIDROrder($condition_data['user_ip_ranges']);

		foreach ($ip_ranges as $ip_range) {
			if (Condition_User_IP_Edit_Fns::_isInSubnet($remote_ip, $ip_range['ip_address'], $ip_range['subnet'])) {
				$grant = $ip_range['grant'];
			}
		}

		return $grant;

	}//end evaluate()


	/**
	* Get a list of match keywords
	*
	* For User IP condition, we are not using the true/false condition - we are
	* using our own Grant/Deny logic. Therefore we will limit the 'match'
	* condition to just the TRUE condition
	*
	* @param string	$type_code	the type code of this asset
	*
	* @return array
	* @access public
	* @static
	*/
	function getMatchKeywords($type_code)
	{
		$match_options = Array(
							'1'	=> translate($type_code.'_true'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>