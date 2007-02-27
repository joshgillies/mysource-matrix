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
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Create Design Customisation
*
* A trigger action that creates a design customisation of the firing asset
*
*
* @author   Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Design_Customisation extends Trigger_Action
{


    /**
    * Execute this action
    * Returns an array of data about what it did, or false on error
    * The settings used by this action are in the form:
    * <PRE>
    * Array(
    *       ''                => int,     // the assetid of the role
    *       );
    * </PRE>
    *
	* (All we need is just the $state['asset'], we clone a design from there, morpth it and get the links.
	*
    * @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
    *
	* @return mixed array|boolean
    * @access public
    */
    function execute($settings, &$state)
    {

		if (empty($state['asset'])) {
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}


		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$clone =& $GLOBALS['SQ_SYSTEM']->am->cloneAsset($asset, $create_link, $map, Array('attributes', 'permissions', 'roles'), FALSE);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$new_name = 'Customisation #'.$clone->id;

		// Acquire a lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Now let's morph the dupe into a design customisation
		if (!$clone->morph('design_customisation')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// and link it back to ourselves
		if (!$asset->createLink($clone, SQ_LINK_TYPE_2, 'customisation', NULL, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		if (!($clone->setAttrValue('id_name', $new_name) && $clone->saveAttributes())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach ($da_links as $link) {
			$da =& $am->getAsset($link['minorid'], $link['minor_type_code'], TRUE);
			if (is_null($da)) continue;
			if (!$clone->createLink($da, SQ_LINK_TYPE_3, $link['value'], NULL, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}//end foreach

		// set an initial web path
		$initial_path = strtolower($new_name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($asset, $valid_paths, $clone->id, TRUE);
		if (!$clone->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// link up all the files from this design to the customisation
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$minor =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$clone->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0162', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// link up all the css files from this design to the customisation
		$css_link_ids = Array();
		$css_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'design_css', FALSE);
		foreach ($css_links as $link) {
			$minor =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$clone->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0161', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// release the lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return	Array(
					'assetid'			=> $clone->id,
					'original_design'	=> $asset->id,
				);

	}//end execute()


    /**
    * Get the HTML editing interface for this action. Returns the string instead of printing it
    *
    * @param array		$settings		settings that this condition saves in processInterface()
    * @param string		$prefix			unique prefix
    * @param boolean	$write_access	determines whether the interface is editable
    *
	* @return boolean
    * @access public
    */
    function getInterface($settings, $prefix, $write_access=FALSE)
    {
		$contents = translate('trigger_create_design_customisation_usage');

		return $contents;

	}//end getInterface()


    /**
    * Handle the conversion of interface to settings
    * together with settings it is expected to populate the hash object
    *
	* @param array	&$settings		a container for any data the action might want to save
    * @param string	$request_data	array of data corresponding to this action as specified
    *								by the $prefix in getInterface.
    *
	* @return boolean
    * @access public
    */
    function processInterface(&$settings, $request_data)
    {
        // make sure the assetid isn't blank

        return FALSE;

	}//end processInterface()


    /**
    * Get the list of locks that the action needs to acquire before executing
    *
    * @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
    *
	* @return array
    * @access public
    */
    function getLocks($settings, &$state)
    {
        return	Array (
					$state['assetid']	=> Array('all'),
				);

    }//end getLocks()


}//end class


?>
