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
* $Id: data_record_edit_fns.inc,v 1.4 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Data_Record_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_View_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	 * Paint the "overall" report.
	 *
	 */
	public function paintSummaryOverallReport($asset, &$o, $prefix)
	{
	    $months = 6;
	    $start_date = strtotime('-'.$months.' months 00:00:00');
	    $end_date   = strtotime('yesterday 00:00:00');
	    $day_count  = round(1 + (($end_date - $start_date) / 86400), 0);
	    $report = $asset->generateReport(
	        Array(),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $report = array_get_index($report, 0, Array());
	    
	    // Give us some goal stats for this period, five at a time (because
	    // 10 metrics maximum).
	    $goals = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->bridgeid)->getGoals($asset->viewid);
	    $goals_reports = Array();
	    foreach (array_chunk(array_keys($goals), 5) as $goalids) {
	        $goal_metrics = Array();
	        foreach ($goalids as $goalid) {
	            $goal_metrics[] = 'ga:goal'.$goalid.'Starts';
	            $goal_metrics[] = 'ga:goal'.$goalid.'Completions';
	        }
	        $goals_report = $asset->generateReport(
                Array(),
                $goal_metrics,
                $start_date,
                $end_date
	        );
	        $goals_reports = array_merge($goals_reports, $goals_report[0]);
	    }//end foreach
	    
	    // But we still need to calculate our own rate, because Starts only
	    // has meaning for a funnel. Anything that doesn't have a funnel will
	    // have the same number of starts as completions, so reset that to
	    // the number of sessions.
	    foreach ($goals as $goal) {
	        if (($goal['type'] !== 'URL_DESTINATION') || ($goal['urlDestinationDetails']['firstStepRequired'] === FALSE)) {
	            $report['ga:goalStartsAll'] -= $goals_reports['ga:goal'.$goal['id'].'Starts'];
	            $report['ga:goalStartsAll'] += $report['ga:sessions'];
	            
	            $goals_reports['ga:goal'.$goal['id'].'Starts'] = NULL;
	            if ($report['ga:sessions'] === 0) {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = NULL;
	            } else {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = $goals_reports['ga:goal'.$goal['id'].'Completions'] / $report['ga:sessions'];
	            }
	        } else {
	            if ($goals_reports['ga:goal'.$goal['id'].'Starts'] === 0) {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = NULL;
	            } else {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = $goals_reports['ga:goal'.$goal['id'].'Completions'] / $goals_reports['ga:goal'.$goal['id'].'Starts'];
	            }
	        }
	    }//end foreach
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    // Sessions and unique users.
	    $o->openSection(translate('Sessions and users'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total sessions') ?></th>
	            <td><strong><?php echo number_format($report['ga:sessions'], 0); ?></strong></td>
	            <td><?php echo translate('%s / day', number_format($report['ga:sessions'] / $day_count, 0)); ?></td>
	            <td><?php echo translate(
	                '%s (%s%%) bounces',
	                number_format($report['ga:bounces'], 0),
	                number_format((100.0 * $report['ga:bounces'] / $report['ga:sessions']), 1)
                ); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Unique users') ?></th>
	            <td><strong><?php echo number_format($report['ga:users'], 0); ?></strong></td>
	            <td colspan="2"><?php echo translate('%s / day', number_format($report['ga:users'] / $day_count, 0)); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('New users') ?></th>
	            <td colspan="3"><?php echo number_format($report['ga:newUsers'], 0); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Sessions per user') ?></th>
	            <td colspan="3"><?php echo number_format(($report['ga:sessions'] / $report['ga:users']), 2); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Return visits per user') ?></th>
	            <td><?php echo number_format(($report['ga:sessions'] / ($report['ga:users'] - $report['ga:newUsers']) - 1), 2); ?></td>
	            <td colspan="2"><?php echo translate('Excludes first visit from a new user during this period'); ?></td>
	        </tr>
	    </tbody></table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    // Session statistics.
	    $o->openSection(translate('Session statistics'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total page views') ?></th>
	            <td><strong><?php echo number_format($report['ga:pageviews'], 0); ?></strong></td>
	            <td colspan="2"><?php echo translate('%s / day', number_format($report['ga:pageviews'] / $day_count, 0)); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Page views per session') ?></th>
	            <td><?php echo number_format($report['ga:pageviews'] / $report['ga:sessions'], 1); ?></td>
	            <td colspan="2"><?php echo translate(
	                '%s excluding bounces',
	                number_format($report['ga:pageviews'] / ($report['ga:sessions'] - $report['ga:bounces']), 1)
	            ); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Session duration') ?></th>
	            <td><?php 
	            $time = $report['ga:sessionDuration'] / $report['ga:sessions'];
                    if ($time > 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), ($time % 60));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), (floor($time * 10) / 10));
                    }
	            ?></td>
	            <td colspan="2"><?php
	                $nb_time = $report['ga:sessionDuration'] / ($report['ga:sessions'] - $report['ga:bounces']);
                    if ($nb_time > 60) {
                        $nb_str = sprintf(translate('%s min %s sec', '%s', '%s'), (int)($nb_time / 60), ($nb_time % 60));
                    } else {
                        $nb_str = sprintf(translate('%s sec', '%s'), (floor($nb_time * 10) / 10));
                    }
                        
                    echo translate('%s excluding bounces', $nb_str);
	            ?></td>
	        </tr>
	    </tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    // Start the section on goal stats.
	    $o->openSection(translate('Goal statistics'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="30%" />
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <thead>
	        <tr>
	            <th><?php echo translate('Goal name') ?></th>
	            <th><?php echo translate('Goal type') ?></th>
	            <th><?php echo translate('Completion ratio') ?></th>
	            <th><?php echo translate('Completions') ?></th>
	            <th><?php echo translate('Starts') ?></th>
	        </tr>
	    <tbody><?php
	        if (count($goals) === 0) {
	            ?><tr>
	            <td colspan="5"><?php echo translate('No goals have been created for this view'); ?></td>
	            </tr><?php
	        } else {
                ?><tr>
                    <th><?php echo translate('All goals') ?></th>
                    <td></td>
                    <td><strong><?php
                        if ($report['ga:goalStartsAll'] > 0) {
                            echo number_format($report['ga:goalCompletionsAll'] / $report['ga:goalStartsAll'] * 100.0, 1).'%';
                        } else {
                            echo translate('N/A');
                        }//end if
                    ?></strong></td>
                    <td><strong><?php echo number_format($report['ga:goalCompletionsAll'], 0); ?></strong></td>
                    <td><strong><?php echo number_format($report['ga:goalStartsAll'], 0); ?></strong></td>	           
                </tr><?php
                foreach ($goals as $goal) {
                ?><tr>
                    <th><?php echo $goal['id'].'. '.$goal['name'] ?></th>
                    <td><?php
                        switch ($goal['type']) {
                            case 'URL_DESTINATION':
                                if ($goal['urlDestinationDetails']['firstStepRequired'] === TRUE) {
                                    echo translate('URL (Funnel)');
                                } else {
                                    echo translate('URL (Single Page)');
                                }
                            break;
                            case 'EVENT':
                                echo translate('Event Tracking');
                            break;
                            case 'VISIT_NUM_PAGES':
                                echo translate('Page Views');
                            break;
                            case 'VISIT_TIME_ON_SITE':
                                echo translate('Session Duration');
                            break;
                            default:
                                echo translate('Unknown');
                            break;
                        }
                    ?></td>
                    <td><strong><?php
                        if ($goals_reports['sq:goal'.$goal['id'].'Rate'] !== NULL) {
                            echo number_format($goals_reports['sq:goal'.$goal['id'].'Rate'] * 100.0, 1).'%';
                        } else {
                            echo translate('N/A');
                        }//end if
                    ?></strong></td>
                    <td><?php echo number_format($goals_reports['ga:goal'.$goal['id'].'Completions'], 0); ?></td>
                    <td><?php echo number_format(array_get_index($goals_reports, 'ga:goal'.$goal['id'].'Starts', $report['ga:sessions']), 0); ?></td>	           
                </tr><?php
                }//end foreach
            }//end if
	        ?></tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    // Start the section on experiment stats (holy bleep).
	    $experiments = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->bridgeid)->getExperiments($asset->viewid);
	    $o->openSection(translate('Content Experiments'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="30%" />
	    <col width="15%" />
	    <col width="23%" />
	    <col width="8%" />
	    <col width="8%" />
	    <col width="8%" />
	    <col width="8%" />
	    <thead>
	        <tr>
	            <th><?php echo translate('Experiment name and description') ?></th>
	            <th><?php echo translate('Status') ?></th>
	            <th><?php echo translate('Variations') ?></th>
	            <th><?php echo translate('Traffic Coverage') ?></th>
	            <th><?php echo translate('Variation Weighting') ?></th>
	            <th><?php echo translate('Minimum Length') ?></th>
	            <th><?php echo translate('Winner Confidence') ?></th>
	        </tr>
	    <tbody><?php
	        if (count($experiments) === 0) {
	            ?><tr>
	            <td colspan="7"><?php echo translate('No content experiments available for this view'); ?></td>
	            </tr><?php
	        } else {	       
                foreach ($experiments as $experiment) {
                ?><tr>
                    <th style="font-weight: normal"><strong><?php echo $experiment['name'] ?></strong><br/>
                    Target metric: <strong><?php echo $experiment['objectiveMetric'] ?></strong><br/>
                    <?php echo ellipsisize($experiment['description'], 150); ?></th>
                    <td><?php
                        switch ($experiment['status']) {
                            case 'DRAFT':
                                echo translate('Draft');
                            break;
                            case 'READY_TO_RUN':
                                echo translate('Ready to run');
                            break;
                            case 'RUNNING':
                                echo translate('Running');
                            break;
                            case 'ENDED':
                                $end_time = strtotime($experiment['endTime']);
                                if (strtotime('+2 days', $end_time) > time()) {
                                    echo translate('Just ended');
                                } else {
                                    echo translate('Ended %s', date('jS M Y', $end_time));
                                }
                                
                                echo '<br/>';
                                switch ($experiment['reasonExperimentEnded']) {
                                    case 'WINNER_FOUND':
                                        foreach (array_get_index($experiment, 'variations', Array()) as $variation) {
                                            if ($variation['won']) {
                                                echo translate('<strong>%s</strong> won', $variation['name']);
                                                break;
                                            }
                                        }
                                    break;
                                    case 'EXPERIMENT_EXPIRED':
                                        echo translate('Expired before winner found');
                                    break;
                                    case 'ENDED_WITH_NO_WINNER':
                                        echo translate('Ended without a winner');
                                    break;
                                    case 'STOPPED_BY_USER':
                                        echo translate('Stopped by user');
                                    break;
                                    default:
                                        // Do nothing.
                                    break;
                                }
                            break;
                            default:
                                echo translate('Unknown');
                            break;
                        }
                    ?></td>
                    <td><?php
                    $variation_list = Array();
                    foreach (array_get_index($experiment, 'variations', Array()) as $variation) {
                        $variation_str = $variation['name'];
                        if (array_get_index($variation, 'inactive', FALSE) === TRUE) {
                            $variation_str .= ' ('.translate('Inactive').')';
                        }
                        if (array_key_exists('weight', $variation) === TRUE) {
                            $variation_str .= ' ('.number_format($variation['weight'] * 100.0, 0).'%)';
                        }
                        $variation_list[] = $variation_str;
                    }
                    echo '<ol><li>'.implode('</li><li>', $variation_list).'</li></ol>';
                    ?></td>
                    <td><?php echo translate('%s%%', $experiment['trafficCoverage'] * 100.0) ?></td>
                    <td><?php 
                    if ($experiment['equalWeighting'] === TRUE) {
                        echo translate('Equal');
                    } else {
                        echo translate('Dynamic');
                    } ?></td>
                    <td><?php echo translate('%s days', $experiment['minimumExperimentLengthInDays']) ?></td>
                    <td><?php echo translate('%s%%', number_format($experiment['winnerConfidenceLevel'] * 100.0, 1)) ?></td>
                </tr><?php
                }//end foreach
            }//end if
	        ?></tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	}//end paintSummaryOverallReport()


}//end class

?>
