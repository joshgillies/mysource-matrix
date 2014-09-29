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
* $Id: condition_user_ip.inc,v 1.12 2012/10/11 02:38:31 cupreti Exp $
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
* @version $Revision: 1.12 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* @param Asset	$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, Array $condition_data)
	{
		if (!isset($condition_data['user_ip_ranges'])) {
			return FALSE;
		}
		if (!isset($_SERVER['REMOTE_ADDR'])) {
			return FALSE;
		}

		$remote_ip = $_SERVER['REMOTE_ADDR'];
		// This condition deals with ipv4 only, so if the user ip is ipv6, bail out
		if (isIpV6($remote_ip)) {
			return TRUE;
		}

		$grant = array_get_index($condition_data, 'default_grant', FALSE);

		// sort the IP ranges by subnet mask order
		$ip_ranges = Condition_User_IP_Edit_Fns::sortByCIDROrder($condition_data['user_ip_ranges']);

		foreach ($ip_ranges as $ip_range) {
			if (Condition_User_IP_Edit_Fns::isInSubnet($remote_ip, $ip_range['ip_address'], $ip_range['subnet'])) {
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
	public static function getMatchKeywords($type_code)
	{
		$match_options = Array(
							'1'	=> translate('The user\'s IP address matches the following pattern'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>
