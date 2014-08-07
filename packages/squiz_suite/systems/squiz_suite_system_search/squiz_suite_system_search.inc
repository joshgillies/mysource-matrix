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
* $Id: squiz_suite_system_search.inc,v 1.7 2012/08/30 00:59:25 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/squiz_suite/systems/squiz_suite_system/squiz_suite_system.inc';

/**
* Squiz_Suite_System_Search
*
* This system provides method that any registered Squiz Suite product can call on. 
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Squiz_Suite_System_Search extends Squiz_Suite_System
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
		$this->_ser_attrs = TRUE;
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a list of sitemap xml for use with Funnelback. 
    *
    * @param array $urls     An array of urls to get the sitemap for.
    * @param array $excludes An array of urls to exclude from the sitemap.
    *
    * @return string
    * @access public
    */
    public function getSitemapXml($urls, $excludes)
    {
        $xml   = Array();
        $fb    = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
        $suite = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		$info  = $suite->getCurrentProduct();
		$systemid = array_get_index($info, 'systemid', NULL);
		if ($systemid === NULL) {
			return '';
		}//end if
        
        // Remove duplicate
        $urls = array_unique($urls);
        $excludes = array_unique($excludes);
        
        foreach ($urls as $single_url) {
            $protocol = parse_url($single_url, PHP_URL_SCHEME);
            $real_url = str_replace($protocol.'://', '', $single_url);
        
            $asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $real_url, TRUE, TRUE);
        
            // Asset not found, skip over
            if (is_null($asset)) continue;
        
            // Pass into Funnelback
            $collected_xmls = $fb->getXMLCache(Array($single_url), $asset->id, FALSE, $excludes);
        
            foreach ($collected_xmls as $collect_xml) {
                $content = array_get_index($collect_xml, 'xml', '');
                if (empty($content)) continue;
        
                $xml[] = $content;
            }//end foreach
        }//end foreach

        // Remove duplicates
        $xml = array_unique($xml);

		// Add in the Product Key for SquizSuite
		foreach ($xml as $index => $piece) {
            if ($systemid !== NULL) {
                // If the requesterid is set, then we must add the requesterid on the fly.
                preg_match('/<fbmeta name="access_locks" content="(.*?)" \/>/i', $piece, $matches);

                if (isset($matches[1])) {
                    preg_match_all('/\'(.*?)(\'\,|\'$)/i', $matches[1], $userid_matches);
                    if (isset($userid_matches[1])) {
                        $userids      = $userid_matches[1];
                        $real_userids = array();
                        foreach ($userids as $userid) {
                            if ($userid == 'public') {
                                $real_userids[] = "'public'"; 
                            } else {
                                $real_userids[] = "'".$systemid.'://'.$userid."'";
                            }//end if
                        }//end foreach

                        $real_userids_string = implode(',', $real_userids);
                        
                        // Finally, replace the corrected string.
                        $pattern = '/<fbmeta name="access_locks" content="(.*?)" \/>/i';
                        $replace = '<fbmeta name="access_locks" content="'.$real_userids_string.'" />';
                        $piece   = preg_replace($pattern, $replace, $piece);
                    }
                }
            }

			$xml[$index] = preg_replace('/<url>(.*?)<\/url>/i', '<url>\\1<fbmeta name="SQUIZPRODUCTID" content="'.$systemid.'" /></url>', $piece);
		}
        
        if (!empty($xml)) {
            return $xml;
        }//end if
        
        throw new Exception('Error: No urls found');

    }//end getSitemapXml()


    /**
    * Returns the page content of a url 
    *
    * @param string $url        The url to get the page content for.
    *
    * @return string
    * @access public
    */
    public function getPageContent($url)
    {
		$contents = '';
		$protocol = parse_url($url, PHP_URL_SCHEME);
		$base_url = str_replace($protocol.'://', '', $url);
		$suite = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromUrl($protocol, $base_url);
		$root_url = $GLOBALS['SQ_SYSTEM']->am->getRootURL($base_url);

		// flag to see if the context is changed to anything but default
		$context_changed = FALSE;
		if (isset($root_url['base_contextid']) && $root_url['base_contextid'] != '0') {
			$GLOBALS['SQ_SYSTEM']->changeContext($root_url['base_contextid']);
			$context_changed = TRUE;
		}

		// Fudge Server Var so designs etc. work
		$old_server_host = $_SERVER['HTTP_HOST'];
		$old_server_self = $_SERVER['PHP_SELF'];
		$_SERVER['HTTP_HOST'] = parse_url($url, PHP_URL_HOST);
		$_SERVER['PHP_SELF']  = parse_url($url, PHP_URL_PATH);

		if ($asset->id !== $suite->id) {
			if ($asset instanceof File) {
				ob_start();
					$existing = $asset->getExistingFile();
					$mime = 'text/plain';
					$filename = $asset->attr('title');
					if (!empty($existing)) {
						require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
						require_once SQ_FUDGE_PATH.'/general/file_system.inc';

						$ext = get_file_type($existing['filename']);
						if (($asset->status & SQ_SC_STATUS_SAFE_EDITING) && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)) {
							// strip file versioning suffix from the file extension, e.g. 'pdf,ffv5' becomes 'pdf'
							$ext = preg_replace('/,ffv\d+$/', '', $ext);
							$existing['filename'] = preg_replace('/,ffv\d+$/', '', $existing['filename']);
						}
						$mime = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];
						$filename = $existing['filename'];
						readfile($existing['path']);
					}
					$contents = ob_get_contents();
				ob_end_clean();

				if (!empty($contents)) {
					$response = Array(
									'_base64'	=> chunk_split(base64_encode($contents)),
									'mimetype'	=> $mime,
									'filename'	=> $filename,
								);
				} else {
					$response = $contents;
				}
			} else {
				ob_start();
					$asset->printFrontend();
					$response = ob_get_contents();
				ob_end_clean();
			}//end if
		}//end if

		if ($context_changed) $GLOBALS['SQ_SYSTEM']->restoreContext();

		$_SERVER['HTTP_HOST'] = $old_server_host;
		$_SERVER['PHP_SELF']  = $old_server_self;

		return $response;
        
        throw new Exception('Error: No urls found');

    }//end getPageContent()


    /**
     * The API function for returning the asset info. 
     *
     * @param array $urls     An array of the URLs to get the info for.
     *
     * @return array
     */
    public static function getAssetInfo(array $urls)
    {
        $info = array();

        foreach ($urls as $real_url) {
            $am = $GLOBALS['SQ_SYSTEM']->am;

            $url_parts = parse_url($real_url);
            $protocol  = array_get_index($url_parts, 'scheme', '');
			if (empty($protocol) === FALSE) {
	            $url = str_replace($protocol.'://', '', $real_url);
			}
            $asset = $am->getAssetFromUrl($protocol, $url, TRUE, TRUE);
            if ($asset !== NULL) {
                $type     = $asset->type();
                $iconPath = $am->getAssetIconURL($type);

                $info[$real_url] = array(
                                    'assetid'  => $asset->id,
                                    'name'     => $asset->attr('name'),
                                    'icon'     => $iconPath,
                                   );
                $am->forgetAsset($asset, TRUE);
                unset($asset);
            } else {
                $info[$real_url] = NULL;
            }
        }//end foreach

        return $info;

    }//end getAssetInfo()


    /**
     * The API function for returning the following for the passed url of the file type asset:
     *      - metadata information
     *      - security keys (see funnelback_manager.inc::getAccessLocks)
     *      - author name, Matrix file asset title, actualy file name
     *
     * @param string/array	$urls	An array of the URLs to get the info for.
     *
     * @return json
     */
    public static function getFileIndexingComponents($urls)
    {
        $info = Array();
		if (!is_array($urls)) $urls = Array($urls);

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager', TRUE);

        foreach ($urls as $real_url) {

            $url_parts = parse_url($real_url);
            $protocol  = array_get_index($url_parts, 'scheme', '');
			if (empty($protocol) === FALSE) {
	            $url = str_replace($protocol.'://', '', $real_url);
			}

            $asset = $am->getAssetFromUrl($protocol, $url, TRUE, TRUE);
            if ($asset !== NULL && strpos($url, '__data') !== FALSE) {
				// found the asset?
				// get all we want
				$metadata = $mm->getMetadata($asset->id);
				foreach ($metadata as $meta) {
					$name = $meta[0]['name'];

					$value = $meta[0]['value'];
					require_once SQ_FUDGE_PATH.'/general/text.inc';
					$keywords = retrieve_keywords_replacements($value);
					$replacements = Array();
					foreach ($keywords as $keyword) {
						$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
					}//end foreach
					replace_keywords($value, $replacements);
					$name = $fm->encodeText(htmlspecialchars($name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
					$value = $fm->encodeText(htmlspecialchars($value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
					$info[$real_url]['metadata'][$name] = $value;
				}//end foreach

				// get the security keys
				$info[$real_url]['security_locks'] = $fm->getAccessLocks($asset);

				$info[$real_url]['SQUIZASSETYPE'] = $asset->type();
				$info[$real_url]['last_modified'] = date('Y-m-d', $asset->updated).'T'.date('H:i:s', $asset->updated);

				// get author, title, file name
				$info[$real_url]['file_title'] = $asset->attr('title');
				$info[$real_url]['file_name'] = $asset->name;
				if ($asset->created_userid != 0) {
					$created_by = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->created_userid);
					$info[$real_url]['author_name'] = $created_by->name;
					$info[$real_url]['author_id'] = $created_by->id;
				}
				$info[$real_url]['created'] = date('Y-m-d', $asset->created).'T'.date('H:i:s', $asset->created);


                $am->forgetAsset($asset, TRUE);
                unset($asset);
            } else {
                $info[$real_url] = NULL;
            }
        }//end foreach

        return json_encode($info);

    }//end getFileIndexingComponents()


    /**
     * The API function for Funnelback system to activate (set LIVE)
     * or deactivate (set UNDER CONSTRUCTION) the funnelback user configured on funnelback manager
     *
     * @param string	$userid	Id of user funnelback manager is configured
     * @param string	$set_to	activate/deactivate string
     *
     * @return boolean
     */
    public static function changeStatusFunnelbackUser($userid, $set_to='activate')
    {
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$status = SQ_STATUS_UNDER_CONSTRUCTION;
		if ($set_to != 'activate' && $set_to != 'deactivate') {
			return Array('success' => FALSE);
		} else {
			if ($set_to == 'activate') $status = SQ_STATUS_LIVE;
		}
		// if the userid isnt provided
		// or it doesn't exist on system
		// return FALSE but no errors
		if (empty($userid)) return Array('success' => FALSE);
		if (!$am->assetExists($userid)) return Array('success' => FALSE);

		// passed userid doesnt match 
		// the user configured return false
		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager', TRUE);
		if ($fm->attr('user') != $userid) return Array('success' => FALSE);

		// if the user is already live, less work for us
		$fb_user = $am->getAsset($userid);
		if ($fb_user->status === $status) return Array('success' => TRUE);

		// all checks done and now we sure we need to 
		// change the status of the user asset passed
		require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_edit_status.inc';
		$init_hipo = new HIPO_Job_Edit_Status();
		$running_vars = Array(
                            'assetid'           => $fb_user->id,
                            'new_status'        => $status,
                            'dependants_only'   => 0,
                        );

		$init_hipo->setRunningVars($running_vars);
		$init_hipo->_steps = $init_hipo->getInitialStepData();
		if (!$init_hipo->prepare()) {
			error_log('Could not initialise HIPO job');
			return Array('success' => FALSE);
		}
		$init_hipo->freestyle();
		
		$errors = $init_hipo->getErrors();
		if(!empty($errors)) {
			error_log($errors);
			return Array('success' => FALSE);
		}

		return Array('success' => TRUE);

	}//end activateFunnelbackUser()


    /**
     * The API function to validate if the user is active or not by
     * passing in the username and optionally password. If the flag
     * to return user keys is set will return array or returns boolean
     *
     * @param string	$username		username to validate
     * @param string	$password		password to validate the username against (optional)
     * @param boolean	$return_keys	flag option if userkeys are to be returned (optional)
     *
     * @return mixed
     */
    public static function validateActiveUser($username, $password='', $return_keys=FALSE)
    {
		if (empty($username)) return FALSE;
		$res = FALSE;

		$user = NULL;
		// Get a list of all the installed authentication systems
		$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}

		$auth_systems = $auth_folder->getAuthSystems();
		foreach ($auth_systems as $systemid) {
			$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
			if (is_null($system)) continue;

			if (empty($password)) {
				$user = $system->locateUser($username);
			} else {
				// okie seems like we are passed a password too
				$user = $system->authenticateUser($username, $password);
			}

			if (!is_null($user)) {
				$res = TRUE;
				break;
			}
		}//end forach

		// okie so we have found the user and we want to sent back
		// the user id and id of all the groups it belongs to
		if ($return_keys && $res) {
			$res = array_merge(Array($user->id), $user->getUserGroups());
		}

		return $res;

	}//end activateFunnelbackUser()


}//end class

?>
