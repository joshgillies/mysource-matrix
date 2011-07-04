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
*$Id: performance_manager.inc,v 1.2 2011/07/04 01:24:07 ewang Exp $
*
*/




/**
* Performance_Manager
*
* Purpose
*    Manages performance mode requests. Start timer, stop timer, paint results.
*    Timer start,stop code has to be inserted to the asset class where performance data to be tracked.
*    Within same asset, timers can not be nested. Between different assets, timer can be nested as asset can be nested, printed mutiple times. 
*    
*     
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Performance_Manager extends MySource_Object
{
	
	// temperary holder for all performance calculation data
	public $performance_data;
	
	
	/**
	* Constructor
	*
	* initialize performance data used to store all performance metrics results
	* initialize timer stack used to calculating orders of timer on the run
	* 
	* @return void
	* @access public
	*/
	function __construct()
	{
		if(!defined('SQ_IN_PERFORMANCE_TIMING')) return;
		$this->performance_data = Array();
		// timer calling stack, so we can know the order of nested timers and calculate active time
		$this->timer_stack = Array();
		return;
		
	}//end constructor
	
	
	/**
	* Start total timer
	*
	* @return void
	* @access public
	*/
	function begin()
	{
		if(!defined('SQ_IN_PERFORMANCE_TIMING')) return;
		unset($this->performance_data);
		unset($_SESSION['SQ_PERFORMANCE_DATA']);
		
		$this->performance_data['total_start'] = microtime(TRUE);
	}
	
	/**
	* End total timer
	*
	* @return void
	* @access public
	*/
	function end()
	{
		if(!defined('SQ_IN_PERFORMANCE_TIMING')) return;
		$this->performance_data['total_end'] = microtime(TRUE);
		
		// record rest of queries
		if(!isset($this->performance_data['total_num_queries'])) {
			$this->performance_data['total_num_queries'] = 0;
		}
		$this->performance_data['total_num_queries'] += count(DAL::getLoggedQueries());
		
		// save all data to session
		$_SESSION['SQ_PERFORMANCE_DATA'] = $this->performance_data;
	}
	
	/**
	* Start asset timer
	*
	* @param mixed		$asset		current asset to be timed
	* @param string		$step		an asset can be timed for a number of steps. no need to specify step if there is only one step
	* 
	* @return void
	* @access public
	*/
	function startTimer($asset, $step = 'default')
	{
		
		if(!defined('SQ_IN_PERFORMANCE_TIMING')) return;
		$assetid = $asset->id;
		$name = $asset->name;
		$type_code = $asset->type();
		if(empty($assetid) || empty($name) || empty($type_code)) return;
		
		
		$time = microtime(TRUE);
		$this->performance_data[$assetid]['name'] = $name;
		$this->performance_data[$assetid]['type_code'] = $type_code;
		$this->performance_data[$assetid]['data'][$step][] = Array (
										'start' 		=> $time,
										'active' 		=> TRUE,
										'bar_start'	=> TRUE,
									);
		// record num of queries
		if(!isset($this->performance_data['total_num_queries'])) {
			$this->performance_data['total_num_queries'] = 0;
		}
		$num_queries = count(DAL::getLoggedQueries());
		$this->performance_data['total_num_queries'] += $num_queries;
		
									
		// if there is parent timer in stack
		if(!empty($this->timer_stack)) {
			$parent_timer = $this->timer_stack[count($this->timer_stack) - 1];
			$parent_assetid = $parent_timer['assetid'];
			$parent_step = $parent_timer['step'];
			$last_position = count ($this->performance_data[$parent_assetid]['data'][$parent_step]) - 1;
			$last_position_parent =& $this->performance_data[$parent_assetid]['data'][$parent_step][$last_position];
			
			// close off active timer of parent
			$last_position_parent['end'] = $time;
			$last_position_parent['duration'] = round($time - $last_position_parent['start'], 6);
			$last_position_parent['num_queries'] = $num_queries;

			// start idle timer of parent
			$this->performance_data[$parent_assetid]['data'][$parent_step][] = Array (
								'start'		=> $time,
								'active'	=> FALSE,
							);
		}
		
		// clear query stack
		DAL::clearLoggedQueries();
		
		// add current timer to stack
		$this->timer_stack[] = Array (
						'assetid'	=> $assetid,
						'step'		=> $step,
						'start'		=> $time,
					);
		return;

	}//end startTimer()
	
	/**
	* Stop asset timer
	* 
	* @param mixed		$asset		current asset to be timed
	* @param string		$step		an asset can be timed for a number of steps. no need to specify step if there is only one step
	* 
	* @return void
	* @access public
	*/
	function stopTimer($asset, $step = 'default')
	{
		if(!defined('SQ_IN_PERFORMANCE_TIMING')) return;
		$assetid = $asset->id;
		$name = $asset->name;
		$type_code = $asset->type();
		if(empty($assetid) || empty($name) || empty($type_code)) return;
		
		$time = microtime(TRUE);
		$last_timer =& $this->performance_data[$assetid]['data'][$step][count($this->performance_data[$assetid]['data'][$step]) - 1];
		$last_timer['end'] = $time;
		$last_timer['duration'] = round($last_timer['end'] - $last_timer['start'], 6);
		$last_timer['bar_end'] = TRUE;
		// record number of queries
		$last_timer['num_queries'] = count(DAL::getLoggedQueries());
		$this->performance_data['total_num_queries'] += $last_timer['num_queries'];
		DAL::clearLoggedQueries();
		
		
		$current_stack_timer = array_pop ($this->timer_stack);

		if(!empty($this->timer_stack)) {
			$parent_timer = $this->timer_stack[count($this->timer_stack) - 1];
			$parent_assetid = $parent_timer['assetid'];
			$parent_step = $parent_timer['step'];
			$last_position = count ($this->performance_data[$parent_assetid]['data'][$parent_step]) - 1;
			$last_position_parent =& $this->performance_data[$parent_assetid]['data'][$parent_step][$last_position];
			
			$last_position_parent['end'] = $time;
			$last_position_parent['duration'] = round($time - $last_position_parent['start'], 4);
			
			// start active timer of parent
			$this->performance_data[$parent_assetid]['data'][$parent_step][] = Array (
								'start'		=> $time,
								'active'	=> TRUE,
							);

		}
		

		return;
	}//end stopTimer()

	
	

	
	/**
	* Print the landing page of performance result
	* It splits to 2 frames for original page and performance result page
	* 
	* @param mixed		$asset		frontend asset
	* @return void
	* @access public
	*/
	public function printFrames($asset)
	{
		$primary_url = current_url();
		unset($_SESSION['SQ_PERFORMANCE_DATA']);
		
		// save switch setting to session
		if(isset($_GET['performance_setting_caching'])) {
			$_SESSION['SQ_PERFORMANCE_SETTING']['caching'] = $_GET['performance_setting_caching'] == 'Default' ? TRUE : FALSE;
			unset($_GET['performance_switch_caching']);
		}
		
		// make up original page url
		$param_string = '?';
		foreach($_GET as $name => $value) {
			$param_string .= $name.'='.$value.'&';
		}
		$param_string .= 'SQ_ACTION=performance_timing';
		
		// make up result page url
		$result_url = $primary_url.'?&SQ_ACTION=performance_result';

		// title
		$title =  $asset->name . ' - Squiz Matrix Performance Mode';
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Frameset//EN" "http://www.w3.org/TR/html4/frameset.dtd">
<html style="height: 100%;"><head>
<meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
<HEAD>
<TITLE><?php echo $title; ?></TITLE>
<SCRIPT LANGUAGE="JavaScript">
<!-- Begin
// set page frame as blank first, then loaded with page url
// this workaround is making sure waiting screen appears first in IE
function fillFrame() {
		parent.content_frame.location.href = "<?php echo ($primary_url.$param_string); ?>";
}



//  End -->
</script>
</HEAD>
<frameset name="frames" rows="90%,350" frameborder="1" framespacing="2" border="3" bordercolor=#000 onLoad="fillFrame();">
  <frame name="content_frame" id='content_frame' src="about:blank"/>
  <frame name="result_frame" id='result_frame' src="<?php echo $result_url; ?>" />
</frameset>
</HTML>
<?php 
	}// end printFrames()

	
	
	
	
	
	/**
	* Print the performance result page
	* note: if there is any index missing in the performance data, check if all open timers are closed properly
	* 
	* 
	* @return void
	* @access public
	*/
	public function printResults()
	{	
		require_once SQ_DATA_PATH.'/private/conf/performance_config.inc';
		

		// if performace data in the other frame is not ready, wait for it
		if(!isset($_SESSION['SQ_PERFORMANCE_DATA'])) {
			$this->_printWaitingScreen();
			return;
		}
		
 		$this->performance_data = $_SESSION['SQ_PERFORMANCE_DATA'];
 		
 		$caching_status = isset($_SESSION['SQ_PERFORMANCE_SETTING']['caching']) && $_SESSION['SQ_PERFORMANCE_SETTING']['caching'] === FALSE ? 'Off' : 'Default';
		$caching_status_reverse = $caching_status === 'Default' ? 'Off' : 'Default';
 		
 		$slowest = Array();
		$active_total = 0;
		foreach ($this->performance_data as $assetid => $content) {
				if(!is_numeric($assetid)) continue;
				$total = 0;
				$num_queries = 0;
				$i = 0;
				foreach ($content['data'] as $step => $timers) {
					foreach ($timers as $index => $timer) {
						if($timer['active'])
							$total += $timer['duration'];
						if(isset($timer['num_queries']))
							$num_queries += $timer['num_queries'];
					}
					$i++;
				}
				// calc slowest assets for summary
				$slowest[$assetid] = Array('time' => $total, 'num_queries' => $num_queries);
				// calc active total time for system time
				$active_total += $total;
				
				// combine multiple steps to a all step for later rendering
				if($i > 1) {
					$this->performance_data[$assetid]['data']['all'] = Array();
					foreach ($content['data'] as $step => $timers) {
						foreach($timers as $timer) {
							$this->_combineSteps($this->performance_data[$assetid]['data']['all'], $timer);
						}
					}
				}
		}
				
		// sort by time
		arsort($slowest);

		$total_time = $this->performance_data['total_end'] - $this->performance_data['total_start'];
		$total_start = $this->performance_data['total_start'];
		$total_assets_count = count($this->performance_data) - 2;
		$total_num_queries = $this->performance_data['total_num_queries'];
		$total_system_time = $total_time - $active_total;
		?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
<link rel="stylesheet" href="<?php echo (sq_web_path('lib').'/web/css/performance.css');  ?>" />
<!--[if gte IE 6]>
    <link rel="stylesheet" href="<?php echo (sq_web_path('lib').'/web/css/performance-ie.css');  ?>" />
<![endif]-->
<script type="text/javascript">
// expand/fold the row where there are performance sub steps
function toggleExpend(id) {

	var rows = getElementsByName_iefix("tr", "row_" + id);
	var len = rows.length;

	 for(i=0, k=0, z=0;i<len;i++) {
		 if(rows[i].style.display == "none") {
			rows[i].style.display="";
		 	k++;
		 	
		 	   for(var j = 0; j < rows[i].childNodes.length; j++)
		 	   {
			 	  var rowSpan =  rows[i].childNodes[j].rowSpan;
			 	  var className =   rows[i].childNodes[j].className;
			 	  var style = rows[i].childNodes[j].style;
			 	  if (className == "asset-perfAction" || className == "asset-perfCell" || style == null) {
				 	  continue;
			 	  }

			 	  if(k == 1) {
				 	  if( rowSpan == 1 && z == 0) {
				 		 rows[i].childNodes[j].rowSpan = len - 1;
				 	  }
				 	 rows[i].childNodes[j].style.display = "";
				 	 if(className == "assetExpand" || className == "assetExpand expanded") {
					 	 if(z == 0) {
					 		rows[i].childNodes[j].setAttribute("class", 'assetExpand expanded'); 
					 	 }
					 	 else {
					 		rows[i].childNodes[j].setAttribute("class", 'assetExpand'); 
					 	 }
				 	 }
			 	  }
			 	  else {
			 		 rows[i].childNodes[j].style.display = "none";
			 	  }
		 	 
		 	   }
		 	 
		 }
		 else {
			 rows[i].style.display="none";
			 z++;
		 }
	 }
}



// IE doesn't support get element by name, so here is workaround
function getElementsByName_iefix(tag, name) {
    
    var elem = document.getElementsByTagName(tag);
    var arr = new Array();
    for(i = 0,iarr = 0; i < elem.length; i++) {
         att = elem[i].getAttribute("name");
         if(att == name) {
              arr[iarr] = elem[i];
              iarr++;
         }
    }
    return arr;
}

/**
* Add a URL parameter (or changing it if it already exists)
* @param {search} string  this is typically document.location.search
* @param {key}    string  the key to set
* @param {val}    string  value 
*/
var addUrlParam = function(search, key, val){
  var newParam = key + '=' + val,
      params = '?' + newParam;

  // If the "search" string exists, then build params from it
  if (search) {
    // Try to replace an existance instance
    params = search.replace(new RegExp('([\?&])' + key + '=[^&]*'), "$1" + newParam);

    // If nothing was replaced, then add the new param to the end
    if (params === search && !search.match(new RegExp('([\?&])' + key))) {
      params += '&' + newParam;
    }
  }

  return params;
};


</script>
</head>

<body>

<div id="perfToolBar">
<div id="perfToolBarPadder">

    <div id="productLogo"><img src="<?php echo (sq_web_path('lib').'/web/images/performance/logo-matrix.png');  ?>" alt="logo-matrix" width="160" height="31" /></div>
    
    <div id="perfSummary">
        <div class="perfTotal">Total Time <strong><?php echo ($this->_printTime($total_time)); ?></strong>,&nbsp; System <strong><?php echo ($this->_printTime($total_system_time)); ?></strong> (<?php echo ($this->_printPercent($total_system_time, $total_time, 2)); ?>), Queries: <strong><?php echo $total_num_queries; ?></strong></div>
        <div><a href="javascript:" onclick="javascript:parent.document.location = parent.document.location.pathname + addUrlParam(parent.document.location.search, 'performance_setting_caching', '<?php echo $caching_status_reverse; ?>');" class="perfSwitch" title="Turn on/off Matrix Caching" target="_top">Caching: <?php echo $caching_status; ?></a></div>
        <div class="perfHighrollers">
            <span class="highRoller-label">Asset count: <strong><?php echo $total_assets_count; ?></strong> Slowest:</span>
            <span class="highRoller-bar">
            <?php 
            $i = 1;
            $top_total_time = 0;
            $top_total_queries = 0;
            foreach ($slowest as $assetid => $data) {
            	$time = $data['time'];
            	$num_queries = $data['num_queries'];
            	// only show top 5 slowest
            	if($i > 5) break;
            	$top_total_time += $time;
            	$top_total_queries += $num_queries;
            	$width = $this->_printPercent($time, $total_time);
            	$time = $this->_printTime($time);
            	$name = $this->performance_data[$assetid]['name'];
            	$slowest_class[$assetid] = 'GUI-colourList-position-'.$i;
            	echo '<span class="highRoller '.$slowest_class[$assetid].'" style="width: '.$width.'" title="'.$name.' '.$time.', '.$num_queries.' queries"><span class="highRoller-details">'.$name.' '.$time.'</span></span>';
            	$i++;
            	
            }
            
            $other_width = $this->_printPercent($total_time - $top_total_time, $total_time);
            $other_time = $this->_printTime($total_time - $top_total_time);
            $other_queries = $total_num_queries - $top_total_queries;
            
            echo '<span class="highRoller GUI-colourList-position-other" style="width: '.$other_width.'" title="Other '.$other_time.', '.$other_queries.' queries"><span class="highRoller-details">Other '.$other_time.', '.$other_queries.' queries</span></span>';
            ?> 
            </div>
        </div>
    </div>
</div>

<div id="perfContents">

<table id="perfTable" cellpadding="0" cellspacing="0" border="0">
<tbody>

<tr>
    <th>Id</th>
    <th>&nbsp;</th>
    <th>Name</th>
    <th>&nbsp;</th>
    <th>Step</th>
    <th>Timeline</th>
</tr>

<?php 
	// print rows
	foreach ($this->performance_data as $assetid => $content) {
			if(!is_numeric($assetid)) continue;

			// grab report from config file
			$report = '';
			if(isset($performance_config[$content['type_code']]['report'])) {
				$report = '<a href="'.$performance_config[$content['type_code']]['report'].'" target="_blank" >';
			}

			$step_count = count($content['data']);
			$expendable = $step_count > 1;
			if($expendable) $step_count = 1;

			$first_row_printed = FALSE;
			foreach($content['data'] as $step => $data) {
				if(count($data) == 0) continue;
				
				
				// calculate total metric for the asset
				$total = 0;
				$total_active = 0;
				$total_num_queries = 0;
				foreach($data as $time_data) {
					$total += $time_data['duration'];
					if($time_data['active'])
						$total_active += $time_data['duration'];
					if(isset($time_data['num_queries']))
				    	$total_num_queries += $time_data['num_queries'];
				}
				$total = $this->_printTime($total);
				$total_active = $this->_printTime($total_active);
				$special_color = isset($slowest_class[$assetid]) ? $slowest_class[$assetid] : '';
				$icon_url = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($content['type_code']);
				
				// print first row
				if($expendable) {
					if($step == 'all') {
						echo '<tr class="expendable" name="row_'.$assetid.'">';
					}
					else {
						echo '<tr class="expendable" style="display:none;" name="row_'.$assetid.'">';
					}					
				}
				else {
					echo '<tr>';
				}
			
?>
					    <td rowspan="<?php echo $step_count; ?>" class="assetID">#<?php echo $assetid; ?></td>
					    <td rowspan="<?php echo $step_count; ?>" class="assetType" style="background-image: url(<?php echo $icon_url; ?>)" title="<?php echo $content['type_code']; ?>" ><img src="<?php echo $icon_url; ?>" alt="<?php echo $content['type_code']; ?>"  width="16" height="16" /></td>
					    <td rowspan="<?php echo $step_count; ?>" class="assetName" title="<?php echo 'Active: '.$total_active.' / Total: '.$total.' / Queries: '.$total_num_queries; ?>"><?php echo !empty($report) ? $report : ''; echo $content['name']; echo !empty($report) ? '</a>' : ''; ?></td>
					    <?php
						if($expendable) {
							echo '<td rowspan="'.$step_count.'" class="assetExpand" ><div class="expandIcon" onClick="toggleExpend('.$assetid.')"></div></td>';
						}
						else {
							echo '<td rowspan="'.$step_count.'" class="assetExpand" >&nbsp;</td>';
						}
						$first_row_printed = TRUE;
		
				    ?>
				    <td class="asset-perfAction" title="<?php echo $step; ?>"><?php echo $step; ?></td>
				    <td class="asset-perfCell">
				    <?php 
				    $offset = 0;
				    foreach($data as $index => $time_data) {
				    	if(isset($time_data['bar_start'])) {
				    		
				    		$bar_total_time = 0;
				    		$bar_active_total_time = 0;
				    		$bar_num_queries = 0;
				    		// start of bar, calculate bar total and paint the wrapper
				    		for ($i = $index, $has_begin = FALSE, $has_end = FALSE; isset($data[$i]) && !($has_begin && $has_end); $i++) {
				    			if(isset($data[$i]['bar_start'])) $has_begin = TRUE;
				    			if(isset($data[$i]['bar_end'])) $has_end = TRUE;
				    			
				    			$bar_total_time += $data[$i]['duration'];
				    			if($data[$i]['active'])
				    				$bar_active_total_time += $data[$i]['duration'];
				    			if(isset($data[$i]['num_queries']))
				    				$bar_num_queries += $data[$i]['num_queries'];
				    		}

				    		$bar_margin_left = $this->_printPercent($time_data['start'] - $total_start - $offset, $total_time);
				    		$bar_width = $this->_printPercent($bar_total_time, $total_time);
				    		$bar_total = $this->_printTime($bar_total_time);
				    		$bar_active_total = $this->_printTime($bar_active_total_time);
				    		
				    		
				    		echo '<div class="perfBar '.$special_color.'" style="margin-left:'.$bar_margin_left.'; width: '.$bar_width.';" title="Active: '.$bar_active_total.' / Total: '.$bar_total.' / Queries: '.$bar_num_queries.'">';
				    		echo '<div style="display:none;">';
				    		echo '</div>';
				    	}
				    	
				    	$block_width = $this->_printPercent($time_data['duration'], $bar_total_time);
				    	$is_dile = $time_data['active'] ? '' : 'idle';
				    	echo '<div class="perfBlock '.$is_dile.'" style="width: '.$block_width.';" ></div>';
				    	
				    	if(isset($time_data['bar_end'])) {
							echo '</div>';
				    	}
				    	
				    	$offset = $time_data['end'] - $total_start;
				    }
				    
				    
				   
				    ?>
				        <div class="perfBar-totals"><?php echo 'Active: '.$total_active.' / Total: '.$total; ?></div>
				    </td>
				</tr>
				
				<?php
			}
			
		}

?>

</tbody>
</table>

</div>

</body>
</html>	
		<?php 
		// end of print, clean the data
		unset($this->performance_data);
		unset($_SESSION['SQ_PERFORMANCE_DATA']);
	}// end printResults()
	

	/**
	* Print the waiting screen in the performance frame
	* 
	* 
	* @return void
	* @access private
	*/
	private function _printWaitingScreen () {
		// refresh every second til performance data is set
		?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html style="height: 100%;">
<head>
<meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
<link rel="stylesheet" href="<?php echo (sq_web_path('lib').'/web/css/performance.css');  ?>" />
<!--[if gte IE 6]>
    <link rel="stylesheet" href="<?php echo (sq_web_path('lib').'/web/css/performance-ie.css');  ?>" />
<![endif]-->
<meta http-equiv="refresh" content="1">
</head>

<body style="height: 100%;">

<div id="perfLoading">
    <span id="loadingMsg">Please wait while performance is analysed ...</span>
</div>
</body>
</html>
		<?php
		return;
	}//end _printWaitingScreen()
	

	/**
	* Print the percentage of time (for css width of block)
	* 
	* @param $time			float		time in seconds to print
	* @param $total_time	float		time in seconds to print
	* @param $precision 	int			round up precision
	* 
	* @return string
	* @access private
	*/
	private function _printPercent ($time, $total_time, $precision = PERFORMANCE_PERCENTAGE_PRINT_PRECISION) {
		$percent = round($time / $total_time, $precision) * 100;
		return $percent.'%';
	}//end _printPercent ()

	
	/**
	* Print the time as string
	* 
	* @param $time			float		time in seconds to print
	* @param $precision 	int			round up precision
	* 
	* @return string
	* @access private
	*/
	private function _printTime ($time, $precision = PERFORMANCE_TIME_PRINT_PRECISION) {
		$time = round($time, $precision);
		return $time.'s';
	}//end _printTime ()
	
	/**
	* Combine multiple sub steps of an asset to single step (all)
	* 
	* @param $all		array		All step
	* @param $timer 	array		individual step to combine
	* 
	* @return void
	* @access private
	*/	
	private function _combineSteps (&$all, $timer) {
		foreach ($all as $index => $current_timer) {
			if ($timer['start'] < $current_timer['end'] || ($timer['start'] == $current_timer['end'] && $timer['end'] < $current_timer['end'])) {
				array_splice($all, $index, count($all), array_merge(Array($timer), array_slice($all, $index))); 
				return;
			}
		}		
		$all[] = $timer;
	}//end _combineSteps()

}//end class

?>
