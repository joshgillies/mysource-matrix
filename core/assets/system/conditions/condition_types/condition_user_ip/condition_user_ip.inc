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
* $Id: condition_user_ip.inc,v 1.6 2006/02/22 04:39:27 lwright Exp $
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
* @version $Revision: 1.6 $
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