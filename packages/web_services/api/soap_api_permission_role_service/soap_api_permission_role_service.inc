<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_permission_role_service.inc,v 1.8 2013/05/07 00:53:08 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Permission_Role_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Permission_Role_Service extends Soap_Api
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array(
				'HasAccess'						=> '1',
				'GetPermission'					=> '1',
				'GetRole'						=> '1',
				'SetPermission'					=> '1',
				'SetRole'						=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation will check whether the requesting user have a certain access level to a particular asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'AssetID'	 		=> [type code of new asset],
	*		 'PermissionLevel'	=> [name for new asset],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function HasAccess($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$access_level	= $this->array_get_index_restricted_value($request_info, 'PermissionLevel', '');

		if (!empty($assetid) && !empty($access_level)) {
			$asset				= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$access_func_call	= strtolower($access_level)."Access";
			$access			= $asset->$access_func_call();
			return Array (
					'HasAccessResult'	=> (bool) $access,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID or Access Level is not valid. Please provide a valid AssetID and Access Level');
		}//end else

	}//end HasAccess()


	/**
	* Description: This operation will set the permission for a user on an asset
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'			=> [The asset in query],
	*		'UserID'			=> [The user which the permission is applying for],
	*		'PermissionLevel'	=> [Read Write Admin],
	*		'Grant'				=> [Apply, Deny, Revoke],
    *		'Cascade'			=> [Whether to cascade permission down to children],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function SetPermission($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$userid				= array_get_index($request_info, 'UserID', '');
		$permission			= $this->array_get_index_restricted_value($request_info, 'PermissionLevel', '');
		$cascade			= self::getBooleanIndex($request_info, 'Cascade', TRUE);
		$grant				= $this->array_get_index_restricted_value($request_info, 'Grant', '');	// Apply, Deny, Revoke = 1, 0, -1
		$grant_level		= Array (
								'Apply'		=> '1',
								'Deny'		=> '0',
								'Revoke'	=> '-1',
							  );

		$grant_val			= $grant_level[$grant];	// We are not going check if isset index, the WSDL should check the input values for $grant
		$perm_level			= self::getPermValue($permission);
		$auto_cascade		= array_get_index($request_info, 'Cascade', '1');

		if (!empty($assetid) && !empty($userid) && !empty($permission)) {
			$hh				= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$running_vars	= Array (
								'permission_changes'	=> Array (
															Array (
																'assetids'			=> Array ( $assetid),
																'granted'			=> $grant_val,
																'permission'		=> $perm_level,
																'userid'			=> $userid,
																'cascades'			=> $auto_cascade,
																'previous_access'	=> NULL,
																'dependants_only'	=> !($cascade && $auto_cascade),
															),
														   ),
							  );
			$errors	= $hh->freestyleHipo('hipo_job_edit_permissions', $running_vars);
			if (empty($errors)) {
				return Array (
						'SetPermissionResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable to set Permission on Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID or Permission Level or User ID is not valid');
		}//end else
	}//end SetPermission()


	/**
	* Description: This operation will return the permission set for on an asset
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'			=> [The asset in query],
	*		'PermissionLevel'	=> [Read Write Admin],
	*		'Granted'			=> [Apply, Deny, Revoke],
	*		'AndGreater'		=> [Check for effective permission (write = read + write)],
	*		'ExpandGroups'		=> [Return userids inside a group instead just groupid],
	*		'AllInfo'			=> [Apply, Deny, Revoke],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetPermission($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$perm_level		= $this->array_get_index_restricted_value($request_info, 'PermissionLevel', '');
		$granted		= self::getNullIndex($request_info, 'Granted', NULL);
		$and_greater	= self::getBooleanIndex($request_info, 'AndGreater', FALSE);
		$expand_groups	= self::getBooleanIndex($request_info, 'ExpandGroups', FALSE);
		$all_info		= self::getBooleanIndex($request_info, 'AllInfo', FALSE);
		$collapse_roles	= self::getBooleanIndex($request_info, 'CollapseRoles', FALSE);

		$perm_level	= self::getPermValue($perm_level);
		if (!empty($assetid) && !empty($perm_level)) {
			$permission_list	= $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, $perm_level, $granted, $and_greater, $expand_groups, $all_info, $collapse_roles);
			$final_result	= Array();
			$i	= 0;
			foreach ($permission_list as $key	=> $value) {
				if ($all_info) {
					$final_result[$i]['UserID']	= $key;
					$final_result[$i]['Grant']	= $value;
				} else {
					$final_result[$i]['UserID']	= $value;
				}//end else
				$i++;
			}//end if
			return Array (
					'GetPermissionResult'	=> $final_result,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID or Access Level is not valid. Please provide a valid AssetID and Access Level');
		}//end else

	}//end GetPermission()


	/**
	* Description: This operation will return an array of roles and the users/groups which can perform it
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'				=> [The assetid of the asset the role is applied to],
	*		'RoleID'				=> [The assetid of the the role that is applied],
	*		'UserID'				=> [The assetid of the user performing the role],
	*		'IncludeAssetID'		=> [Whether to include the assetid in the returned array],
	*		'IncludeGlobals'		=> [Whether to query the role view which includes expanded global roles as individual users],
	*		'IncludeDependants'		=> [If false it will filter out the dependant assets],
	*		'ExpandGroups'			=> [when TRUE, any groups defined within a role will be replaced with the userids in that group]
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetRole($request)
	{
		$request_info		= (Array) $request;
		$assetid			= self::getNullIndex($request_info, 'AssetID', NULL);
		$roleid				= self::getNullIndex($request_info, 'RoleID', NULL);
		$userid				= self::getNullIndex($request_info, 'UserID', NULL);
		$include_assetid	= self::getBooleanIndex($request_info, 'IncludeAssetID', FALSE);
		$include_globals	= self::getBooleanIndex($request_info, 'IncludeGlobals', FALSE);
		$include_dependants	= self::getBooleanIndex($request_info, 'IncludeDependants', TRUE);
		$expand_groups		= self::getBooleanIndex($request_info, 'ExpandGroups', FALSE);

		$role_list	= $GLOBALS['SQ_SYSTEM']->am->getRole($assetid, $roleid, $userid, $include_assetid, $include_globals, $expand_groups, $include_dependants);
		$role_list_result	= Array();
		$i	= 0;

		foreach ($role_list as $role_id	=> $user_info) {
			if ($include_assetid) {
				foreach ($user_info as $userid	=> $assetid_info) {
					$role_list_result[$i]['RoleID']		= $role_id;
					$role_list_result[$i]['UserID']		= $userid;
					$role_list_result[$i]['AssetID']	= $assetid_info[0];
					$i++;
				}//end foreach
			} else {
				if (count($user_info > 1)) {
					foreach ($user_info as $index	=> $userid) {
						$role_list_result[$i]['RoleID']	= $role_id;
						$role_list_result[$i]['UserID']	= $userid;
						$i++;
					}//end foreach
				} else {
					$role_list_result[$i]['RoleID']	= $role_id;
					$role_list_result[$i]['UserID']	= $user_info[0];
				}//end else
			}//end else
			$i++;
		}//end foreach

		return Array (
				'GetRoleResult'	=> $role_list_result,
			   );

	}//end GetRole()


	/**
	* Description: This operation will set role for an user to perform on an asset.
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'				=> [The assetid of the asset the role is applied to],
	*		'RoleID'				=> [The assetid of the the role that is applied],
	*		'UserID'				=> [The assetid of the user performing the role],
	*		'Action'				=> [Add | Delete Role],
	*		'GlobalRole'			=> [Global or non-global role],
	*		'Cascade'				=> [Whether to cascade the role down to children],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function SetRole($request)
	{
		$request_info		= (Array) $request;
		$assetid			= self::getNullIndex($request_info, 'AssetID', NULL);
		$roleid				= self::getNullIndex($request_info, 'RoleID', NULL);
		$userid				= self::getNullIndex($request_info, 'UserID', NULL);
		$action				= self::getNullIndex($request_info, 'Action', NULL);
		$global_role		= self::getBooleanIndex($request_info, 'GlobalRole', FALSE);
		$cascade			= self::getBooleanIndex($request_info, 'Cascade', TRUE);

		if (!empty($assetid) && !empty($roleid)) {
			$hh		= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$action	= strtolower($action);
			if ($global_role) {
				$userid	= 0;
			}//end if
			if ($userid === NULL) {
				throw new SoapFault('Server', 'Please Provide User ID');
			} else {
				// Check that this is a user, but only if it is not global role assignment
				if ($userid !== 0) {
					$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid, Array('user', 'user_group'), FALSE);
					if (empty($user_info)) {
						throw new SoapFault('Server', 'The provided User ID does not belong to a User/User Group');
					}//end if
				}//end if
			}//end

			if ($roleid === NULL) {
				throw new SoapFault('Server', 'Please Provide Role ID');
			} else {
				// Check that this is a role
				$role	= $GLOBALS['SQ_SYSTEM']->am->getAsset($roleid);
				if (!($role instanceof Role)) {
					throw new SoapFault('Server', 'The provided Role ID does not belong to a Role');
				}//end if
			}//end else

			if ($action == 'delete') {
				// Check that there is something to delete
				$user_role	= $GLOBALS['SQ_SYSTEM']->am->getRole($assetid, $roleid, $userid);
				if (empty($user_role)) {
					throw new SoapFault('Server', 'No such Role ID #'.$roleid.' for User ID #'.$userid);
				}//end if
			}//end if

			$role_vars		= Array (
								$action	=> Array (
											$roleid	=> Array (
														$userid,
													   ),
										   ),
							  );
			$running_vars	= Array(
								'assets'		=> Array (
													$assetid	=> Array (
																	'type_code'	=> $asset->type(),
																   ),
												   ),
								'role_changes'	=> $role_vars,
							  );

			$errors	= $hh->freestyleHipo('hipo_job_edit_roles', $running_vars);
			if (empty($errors)) {
				return Array (
						'SetRoleResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable to set Permission on Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID or Access Level is not valid. Please provide a valid AssetID and Access Level');
		}//end else

	}//end SetRole()


	private static function getPermValue($permission)
	{
		$perm_level			= Array (
								'Read'		=> '1',
								'Write'		=> '2',
								'Admin'		=> '3',
							  );

		if (empty($permission) || !isset($perm_level[$permission])) return NULL;

		return $perm_level[$permission];

	}//end getPermValue()


	/**
	* getComplexType
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$role_details				= self::getArgType('RoleDetail', 0, 'unbounded');
		$permission_type			= self::getArgType('PermissionLevel', 1, 1);
		$grant_type					= self::getArgType('Grant', 1, 1);
		$role_action				= self::getArgType('RoleActionType', 1, 1);
		$permission_detail			= self::getArgType('PermissionDetail', 0, 'unbounded');

		$complex_types	= Array (
							'HasAccess'			=> Array (
													'HasAccess'				=> Array (
										    									'AssetID'			=> self::$string_non_optional,
																				'PermissionLevel'	=> $permission_type,
																			   ),
													'HasAccessResponse'		=> Array (
																				'HasAccessResult'	=> self::$boolean_optional,
																			   ),
												   ),
							'SetPermission'	=> Array (
													'SetPermission'			=> Array (
																				'AssetID'			=> self::$string_non_optional,
																				'UserID'			=> self::$string_non_optional,
																				'PermissionLevel'	=> $permission_type,
																				'Grant'				=> $grant_type,
																				'Cascade'			=> self::$boolean_optional,
																			   ),
													'SetPermissionResponse'	=> Array (
																				'SetPermissionResult' => self::$string_optional,
																			   ),
												   ),
							'GetPermission'		=> Array (
													'GetPermission'			=> Array (
																				'AssetID'			=> self::$string_non_optional,
																				'PermissionLevel'	=> $permission_type,
																				'Granted'			=> self::$boolean_optional,
																				'AndGreater'		=> self::$boolean_optional,
																				'ExpandGroups'		=> self::$boolean_optional,
													  							'AllInfo'			=> self::$boolean_optional,
												   								'CollapseRoles'		=> self::$boolean_optional,
																			   ),
													'GetPermissionResponse'	=> Array (
																				'GetPermissionResult' => $permission_detail,
																			   ),
												   ),
							'GetRole'		=> Array (
													'GetRole'				=> Array (
																				'AssetID'			=> self::$string_optional,
																				'RoleID'			=> self::$string_optional,
																				'UserID'			=> self::$string_optional,
																				'IncludeAssetID'	=> self::$string_optional,
																				'IncludeGlobals'	=> self::$string_optional,
													  							'ExpandGroups'		=> self::$string_optional,
												   								'IncludeDependants'	=> self::$string_optional,
																			   ),
													'GetRoleResponse'		=> Array (
																				'GetRoleResult'		=> $role_details,
																			   ),
												   ),
							'SetRole'		=> Array (
													'SetRole'				=> Array (
																				'AssetID'			=> self::$string_non_optional,
																				'RoleID'			=> self::$string_non_optional,
																				'UserID'			=> self::$string_optional,
																				'Action'			=> $role_action,
																				'GlobalRole'		=> self::$boolean_optional,
																			   ),
													'SetRoleResponse'		=> Array (
																				'SetRoleResult'		=> self::$boolean_optional,
																			   ),
												   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;


	}//end getComplexElements();


    /**
    * getComplexTypes: return all the complex types used by the API
    *
    * @return array
    * @access public
    */
    public static function getComplexTypes($func_list=Array())
    {

        $complex_types    = Array (
							'GetRole'			=> Array (
														'RoleDetail'			=> Array (
												   	   							  	 	'RoleID'	=> self::$string_optional,
												   	   								   	'UserID'	=> self::$string_optional,
        																				'AssetID'	=> self::$string_optional,
												   	   								   ),
												   ),
							'GetPermission'		=> Array (
														'PermissionDetail'		=> Array (
												   										'UserID'	=> self::$string_optional,
												   	   							  	 	'Grant'		=> self::$string_optional,
												   	   								   ),
												   ),
                          );

        $complex_types_available = parent::getComplexElements($complex_types);

        return $complex_types_available;

    }//end getComplexTypes()


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{

		$access_level	= Array (
							'restriction_base'	=> 'string',
							'enumeration'		=> Array (
													'Read', 'Write', 'Admin'
												   ),
						  );
		$simple_restricted_types	= Array (
										'HasAccess'		=> Array (
															'PermissionLevel'	=> $access_level,
														   ),
										'SetPermission'	=> Array (
															'PermissionLevel'	=> $access_level,
															'Grant'				=> Array (
																					'restriction_base'	=> 'string',
																					'enumeration'		=> Array (
																											'Apply', 'Deny', 'Revoke',
																										   ),
																				   ),
														   ),
										'SetRole'		=> Array (
															'RoleActionType'	=> Array (
																					'restriction_base'	=> 'string',
																					'enumeration'		=> Array (
																											'Add', 'Delete',
																										   ),
														   						   ),
													   	   ),
						  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>
