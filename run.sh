docker compose -f docker-compose.yml up -d --build pyimax
docker compose -f docker-compose.yml exec pyimax /bin/bash
