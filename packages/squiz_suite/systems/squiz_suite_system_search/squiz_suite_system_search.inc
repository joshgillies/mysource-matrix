<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: squiz_suite_system_search.inc,v 1.4 2011/02/01 05:51:24 bpearson Exp $
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
* @version $Revision: 1.4 $
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


}//end class

?>
