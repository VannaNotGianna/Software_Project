
.DEFAULT_GOAL := help

help: ## Display available commands in Makefile
	@grep -hE '^[a-zA-Z_0-9-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

serve: ## run the server
	docker run --user $(shell id -u):$(shell id -g) -v $(shell pwd):/www/app -w /www/app -p 8000:8000 -it sw_music:1 fastapi run src/app.py

build: ## builds the docker image
	docker build . -f Dockerfile -t sw_music:1 --build-arg uid=$(shell id -u) --build-arg gid=$(shell id -g) --build-arg user=dockeruser --build-arg group=dockergroup

shell:  ## enters a shell attached to the docker image just built
	docker build . -f Dockerfile -t sw_music:1 --build-arg uid=$(shell id -u) --build-arg gid=$(shell id -g) --build-arg user=dockeruser --build-arg group=dockergroup
	docker run -v $(shell pwd):/www/app -w /www/app -it sw_music:1 bash
