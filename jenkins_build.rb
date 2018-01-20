require 'jenkins_api_client'
require 'uri'

$exitCode = 0

if(ARGV[0] == nil) then
	print "No gitlab email address was provided as a command-line argument!"
	exit 1
end

if(not require "./jenkins_credentials.rb") then
	print "Could not include 'jenkins_credentials.rb' script!"
	exit 1
end

$creds = $jenkins_credentials[ARGV[0]]
if($creds == nil) then
	print "No Jenkins credentials provided for '", ARGV[0], "'."
	exit 1 
end
$user = $creds["user"]
$pass = $creds["pass"]


$client = JenkinsApi::Client.new(
		 :server_url => '10.162.37.116:8081', #The fuck does it need port for!?
		 :server_port => 8081,
		 :timeout => 500,
		 :http_open_timeout => 100,
		 :http_read_timeout => 300,
         :username => $user, 
         :password => $pass, 
         :log_level => 2)


class Log
	@@depth = 0

	def self.push()
		@@depth = @@depth + 1
	end


	def self.pop(count=1)
		@@depth = @@depth - count
	end

	def self.write(*args)
		for i in 1..@@depth
			print "\t"
		end

		args.each do |arg|
			print arg
		end
		print "\n"
	end

end

class JenkinsJobManager
	attr_reader :status
	attr_reader :jobsDone
	attr_reader :elapsedTime

	def initialize()
		@jobs 			= []
		@jobsDone		= []
		@done 			= false
		@startTime		= Time.now
		@elapsedTime 	= 0
	end

	def update()

		stillGoing = false

		Log.write("JOB_MANAGER - Updating")
		Log.push


		@jobs.delete_if { |j| 
			status = j.update
			if status == "running" or status == "pending" then
				stillGoing = true
				false
			else
				Log.write("JOB_MANAGER - Removing job ", j.name)
				Log.push
				j.end
				jobsDone.push(j)
				Log.pop
				true
			end
		}
		
		@done = !stillGoing
		Log.write("JOB_MANAGER - ", @done? "Done" : "Still Working")
		Log.pop
		return @done

	end

	def addJob(job)
		Log.write("JOB_MANAGER - Attempting to Add Job ", job.name)

		Log.push

		if job.start then
			@jobs.push(job)
			Log.write("JOB_MANAGER - Job Added ", job.name)
		else
			@jobsDone.push(job)
			Log.write("JOB_MANAGER - Job Not Added ", job.name)

		end

		Log.pop

	end

	def end()
		Log.write("JOB_MANAGER - END")
		Log.push

		@jobs.each { |j| 
			j.end
			jobsDone.push(j)
		}

		Log.pop

		@elapsedTime = (Time.now - @startTime)

		return @jobs.count == 0? true : false
	end 

end


class JenkinsJob
	attr_reader :name
	attr_reader :buildNumber
	attr_reader :buildStatus
	attr_reader :elapsedTime

	@@maxWaitCount 		= 30

	def initialize(build, preCallback=nil, postCallback=nil)
		@name 			= build
		@preBuild		= preCallback
		@postBuild		= postCallback
		@pending		= false
		@pendingCount 	= 0
		@buildResponse	= nil
		@taskId			= nil
		@buildNumber 	= -1
		@buildStatus 	= "CANCELED"
		@startTime		= nil
		@endTime		= nil
	end

	def elapsedTime
		if not @startTime then
			return 0
		end

		return @endTime - @startTime
	end

	def start()

		Log.write("JOB[",@name,"] - Attempting to Start")
		Log.push


		if @preBuild != nil then
			Log.write("Calling prebuild step!")
			Log.push
			if not @preBuild.call(@name) then
				Log.pop(2)
				Log.write("Skipping")
				@buildStatus = "SKIPPED"
				return false
			end
				Log.pop
		end 


		@buildResponse = $client.api_post_request("/job/#{URI.escape(@name.encode(Encoding::UTF_8))}/build", {}, true)


		if not @buildResponse["location"] then
			Log.write("Jenkins did not return a location header in response!")
			Log.pop
			$exitCode = 1
			return false
		end

		task_id_match = @buildResponse["location"].match(/\/item\/(\d*)\//)
		@taskId = task_id_match.nil? ? nil : task_id_match[1]

		if @taskId == nil then
			Log.write("Could not determine a taskId!")
			Log.pop
			$exitCode = 1
			return false
		end

		Log.write("Successfully added to job queue.")
		@pending = true
		Log.pop
		return true
	end


	def update()
		Log.write("JOB[",@name,"] - Updating")
		Log.push

		if @pending == true then
			@pendingCount += 1

			queue_item = $client.queue.get_item_by_id(@taskId)

			if queue_item == nil then
				Log.write("Couldn't even get a queue_item from taskId... SERIOUSLY FUCKED!")
				Log.pop
				$exitCode = 1
				return "FUCKED"
			end

			if queue_item['executable'].nil?
				if @pendingCount >= @@maxWaitCount then
					Log.write("Timed out waiting for build to start. Cancelling...")
					$client.api_post_request("/queue/cancelItem?id=#{@taskId}")
					Log.pop
					$exitCode = 1
					return "FUCKED"

				else 
					# Every 5 attempts (~10 seconds)
					Log.write("Waiting to start (",@pendingCount,"/",@@maxWaitCount,").")
					Log.pop
					return "pending"
				end

			else
				@buildNumber = queue_item['executable']['number']
			
				if @buildNumber == nil then
					Log.write("Job started, but couldn't get build number... seriously fucked.")
					Log.pop
					$exitCode = 1
					return "FUCKED"

				else
					Log.write("Job started successfully with Build #",@buildNumber,".")
					@startTime = Time.now
				end

				@pending = false

			end
		end

		if not @pending then
			@buildStatus = $client.job.get_current_build_status(@name)
			Log.write("Status: ", @buildStatus)
		end

		Log.pop
		return @buildStatus

	end

	def end()
		Log.write("JOB[",@name,"] - Ending")
		Log.push

		@endTime = Time.now

		if @buildNumber != -1 then #bitch never even started!
			if @buildStatus == "running" then
				Log.write("Canceling Build!")
				$client.job.stop_build(@name, @buildNumber)
				@buildStatus = $client.job.get_current_build_status(@name)
			end

			if @postBuild != nil then
				Log.write("Calling postbuild step!")
				Log.push
				@postBuild.call(@name, @buildStatus)
				Log.pop
			end

		end

		Log.pop
	end

end


def vmuChanged()
	if  `git diff --name-only HEAD~1..HEAD libGyro`.split[0] != nil or 
		`git diff --name-only HEAD~1..HEAD ElysianVMU`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD jenkins_build.rb`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD qtcreator`.split[0] != nil then
		return true
	else
		return false
	end
end

def toolkitChanged()
	if  `git diff --name-only HEAD~1..HEAD AssetIO`.split[0] != nil or 
		`git diff --name-only HEAD~1..HEAD ESGamma`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD ESToolkit`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD libGyro`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD Lua5.2`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD OOLua`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD Spine`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD UnitTests`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD jenkins_build.rb`.split[0] != nil or
		`git diff --name-only HEAD~1..HEAD qtcreator`.split[0] != nil then
		return true
	else
		return false
	end
end

vmuChangedPre = lambda { |n|
		if vmuChanged() then
			Log.write("ElysianVMU change detected since last commit!")
			return true

		else
			Log.write("ElysianVMU has not changed since the last commit!")
			return false
		end
	}

toolkitChangedPre = lambda { |n|
	if toolkitChanged() then
		Log.write("ESToolkit change detected since last commit!")
		return true

	else
		Log.write("ESToolkit has not changed since the last commit!")
		return false
	end
}


$mgr = JenkinsJobManager.new

def triggerNextJobLambda(job, success=true)
	return lambda { |n, s| 

		if s == "success" then 
			Log.write(n, " built successfully. Starting next build [",job.name,"].")
			Log.push
			$mgr.addJob(job)
			Log.pop
		elsif not success 
			Log.write(n, " failed, but still triggering next build [",job.name,"].")
			Log.push
			$mgr.addJob(job)
			Log.pop
		else
			Log.write(n, " failed. Not bothering with next build trigger.")
		end
	}

end

buildWin 	= JenkinsJob.new("SeedNFeed_Win_x86")
buildMac	= JenkinsJob.new("SeedNFeed_MacOS")

$mgr.addJob(buildWin)
$mgr.addJob(buildMac)

while $mgr.update == false do
	sleep(1.0) 
end

$mgr.end

Log.write("====== Build Results ======")
Log.push
Log.write("Total Time: ", $mgr.elapsedTime, " (sec)")

$mgr.jobsDone.each { |j| 
	Log.write(j.name, ": ", j.buildStatus.upcase)
	Log.push
	Log.write("Build Number: ", j.buildNumber == -1? "none" : j.buildNumber)
	Log.write("Elapsed Time: ", j.elapsedTime, " (sec)")
	Log.pop
}

Log.pop
Log.write("===========================")

Log.write("Exiting with code: ", $exitCode)

exit $exitCode






